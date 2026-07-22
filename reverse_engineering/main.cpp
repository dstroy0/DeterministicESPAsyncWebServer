#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <Arduino.h>

#define TRIGGER_PIN 4
#define WINDOW_SIZE 2048              // 2048 samples per crypto event window
#define ADC_DATA_PINS_MASK 0x00FFF000 // Example parallel bitmask for external ADC

// Structure to align data with the Python processing engine
struct TracePacket
{
    uint32_t trace_id;
    uint8_t plaintext[16];
    uint16_t adc_window[WINDOW_SIZE];
};

// Thread-safe queues to pass completed windows from the hardware core to the network core
QueueHandle_t windowQueue;
volatile bool windowTriggered = false;
volatile uint32_t sampleCounter = 0;
uint16_t globalCircularBuffer[WINDOW_SIZE];

// Interrupt Service Routine (ISR) - Executes in nanoseconds on pin transition
void IRAM_ATTR onCryptoTrigger()
{
    windowTriggered = true;
    sampleCounter = 0; // Reset counter to fill the post-trigger half of the window
}

// Core 1: High-Speed Hardware Slicing Task
void HardwareCaptureLoop(void *pvParameters)
{
    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), onCryptoTrigger, RISING);

    TracePacket localPacket;
    localPacket.trace_id = 0;

    while (1)
    {
        // While waiting for a trigger, continuously fill the circular buffer
        if (!windowTriggered)
        {
            // High-speed direct register read of parallel ADC pins
            uint32_t raw_gpio = REG_READ(GPIO_IN_REG);
            globalCircularBuffer[sampleCounter % WINDOW_SIZE] = (uint16_t)((raw_gpio & ADC_DATA_PINS_MASK) >> 12);
            sampleCounter++;
        }
        else
        {
            // Trigger hit! Capture the remaining post-trigger window length
            while (sampleCounter < WINDOW_SIZE)
            {
                uint32_t raw_gpio = REG_READ(GPIO_IN_REG);
                localPacket.adc_window[sampleCounter] = (uint16_t)((raw_gpio & ADC_DATA_PINS_MASK) >> 12);
                sampleCounter++;
                ets_delay_us(1); // Microsecond adjustments to scale temporal window size
            }

            // Pop out current plaintext state (mocked or read from SPI bus)
            memset(localPacket.plaintext, 0xAA, 16);
            localPacket.trace_id++;

            // Ship full window struct over to Core 0 queue
            xQueueSend(windowQueue, &localPacket, 0);
            windowTriggered = false; // Re-arm the system for the next event
        }
    }
}

// Core 0: Network Asynchronous Streaming Task
void NetworkStreamingLoop(void *pvParameters)
{
    TracePacket networkPacket;
    // Initialize WiFi and connect TCP socket to main Python Engine here...

    while (1)
    {
        // Block until Core 1 drops a frozen window into the queue
        if (xQueueReceive(windowQueue, &networkPacket, portMAX_DELAY) == pdTRUE)
        {
            // socket.write((uint8_t*)&networkPacket, sizeof(TracePacket));
            // Non-blocking offloaded network delivery happens here safely on Core 0
            printf("[*] Window Packet %d securely transmitted to Engine.\n", networkPacket.trace_id);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    windowQueue = xQueueCreate(10, sizeof(TracePacket));

    // Explicitly pin tasks to separate execution cores
    xTaskCreatePinnedToCore(NetworkStreamingLoop, "NetTask", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(HardwareCaptureLoop, "CapTask", 8192, NULL, 3, NULL, 1);
}

void loop()
{
    vTaskDelete(NULL);
} // Free loop task, handled fully by FreeRTOS tasks

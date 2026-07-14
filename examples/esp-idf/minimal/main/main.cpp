// Minimal ESP-IDF + arduino-esp32 sketch: a one-route web server on the DeterministicESPAsyncWebServer
// library, built with the ESP-IDF CMake toolchain (idf.py) instead of Arduino/PlatformIO.
//
// Arduino autostart is enabled (CONFIG_AUTOSTART_ARDUINO=y in sdkconfig.defaults), so the arduino-esp32
// component calls setup() once and loop() forever - the same shape as an .ino sketch. Set your Wi-Fi
// credentials below and flash with `idf.py flash monitor`.
#include "dwserver.h"
#include "network_drivers/physical/physical.h" // init_wifi_physical / wifi_ready
#include <Arduino.h>
#include <WiFi.h>

static const char *WIFI_SSID = "YOUR_SSID";
static const char *WIFI_PASS = "YOUR_PASSWORD";

DetWebServer server;

static void handle_root(uint8_t slot, HttpReq *)
{
    server.send(slot, 200, "text/plain", "Hello from ESP-IDF + DeterministicESPAsyncWebServer\n");
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    init_wifi_physical(WIFI_SSID, WIFI_PASS);
    Serial.print("WiFi connecting");
    uint32_t t0 = millis();
    while (!wifi_ready() && millis() - t0 < 20000)
    {
        delay(250);
        Serial.print('.');
    }
    if (!wifi_ready())
    {
        Serial.println(" no WiFi");
        return;
    }
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());

    server.on("/", HttpMethod::HTTP_GET, handle_root);
    server.begin(80);
    Serial.println("server ready on :80");
}

void loop()
{
    server.handle();
}

// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// On-device CCOUNT microbenchmark for the SEN0192 motion core (services/sen0192): the pure
// hold/debounce state machine dws_sen0192_motion_update() that turns a raw PIR/microwave OUT level
// into a debounced presence verdict + events. The GPIO read is real-hardware and out of scope.
//
// Build/flash:  pio run -d perf/device/sen0192 -t upload --upload-port COM7
#include "device_bench.h"
#include "services/sen0192/sen0192.h"
#include <Arduino.h>

static void sen0192_bench_task(void *)
{
    for (;;)
    {
        Serial.printf("DB ==== sen0192 device microbench start (CCOUNT @ %u MHz) ====\n",
                      (unsigned)getCpuFrequencyMhz());
        volatile uint32_t sink = 0;
        Sen0192Motion m;
        dws_sen0192_motion_init(&m, 2000, true);
        uint32_t t = 0;
        DBENCH_OP("dws_sen0192_motion_update", 200000, {
            sink += dws_sen0192_motion_update(&m, (t & 128) != 0, t);
            t += 5;
        });
        DBENCH_OP("dws_sen0192_motion_present", 200000, sink += dws_sen0192_motion_present(&m));
        (void)sink;
        Serial.println("DB ==== DONE ====");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println("\nDB boot: sen0192 device microbench");
    xTaskCreatePinnedToCore(sen0192_bench_task, "dbench", 16384, nullptr, 24, nullptr, 1);
}

void loop()
{
    delay(1000);
}

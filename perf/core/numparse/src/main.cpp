// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// On-device CCOUNT microbenchmark for the no-stdlib number parsers (shared_primitives/numparse.h):
// dws_strtol / dws_strtoul / dws_strtof - the hand-rolled base-10 parsers the library uses instead of
// the forbidden libc atoi/strtod, in every codec that reads a number from text. Pure, header-only.
// Build/flash: pio run -d perf/core/numparse -t upload
#include "device_bench.h"
#include "shared_primitives/numparse.h"
#include <Arduino.h>

static void numparse_bench_task(void *)
{
    for (;;)
    {
        Serial.printf("DB ==== numparse device microbench start (CCOUNT @ %u MHz) ====\n",
                      (unsigned)getCpuFrequencyMhz());
        volatile long sink = 0;
        const char *e;
        DBENCH_OP("dws_strtol (-1234567)", 200000, sink += dws_strtol("-1234567", &e));
        DBENCH_OP("dws_strtoul (4000000000)", 200000, sink += (long)dws_strtoul("4000000000", &e));
        DBENCH_OP("dws_strtof (3.14159265)", 200000, sink += (long)(dws_strtof("3.14159265", &e) * 100));
        (void)sink;
        Serial.println("DB ==== DONE ====");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println("\nDB boot: numparse device microbench");
    xTaskCreatePinnedToCore(numparse_bench_task, "dbench", 16384, nullptr, 24, nullptr, 1);
}
void loop()
{
    delay(1000);
}

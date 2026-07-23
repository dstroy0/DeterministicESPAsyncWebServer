// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// On-device CCOUNT microbenchmark for the telemetry primitives (services/telemetry): the rolling
// window statistics (push + mean/variance), the exponential rate estimator, and the trapezoidal
// totalizer. Pure float math; these run per sample on an ingest path.
//
// Build/flash:  pio run -d perf/device/telemetry -t upload --upload-port COM7
#include "device_bench.h"
#include "services/telemetry/telemetry.h"
#include <Arduino.h>

static void telemetry_bench_task(void *)
{
    static float wbuf[32];
    static DWSWindow win;
    static DWSRate rate;
    static DWSTotalizer tot;

    for (;;)
    {
        Serial.printf("DB ==== telemetry device microbench start (CCOUNT @ %u MHz) ====\n",
                      (unsigned)getCpuFrequencyMhz());
        volatile float sink = 0;
        dws_window_init(&win, wbuf, 32);
        for (int i = 0; i < 32; i++)
            dws_window_push(&win, (float)(i % 7) + 0.5f);
        DBENCH_OP("dws_window_push", 200000, dws_window_push(&win, (float)(sink)));
        DBENCH_OP("dws_window_mean", 200000, sink += dws_window_mean(&win));
        DBENCH_OP("dws_window_variance", 200000, sink += dws_window_variance(&win));
        dws_rate_init(&rate);
        uint32_t t = 0;
        DBENCH_OP("dws_rate_update", 200000, {
            sink += dws_rate_update(&rate, sink + 1.0f, t);
            t += 10;
        });
        dws_totalizer_init(&tot);
        t = 0;
        DBENCH_OP("dws_totalizer_add", 200000, {
            sink += (float)dws_totalizer_add(&tot, 2.5f, t);
            t += 10;
        });
        (void)sink;
        Serial.println("DB ==== DONE ====");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println("\nDB boot: telemetry device microbench");
    xTaskCreatePinnedToCore(telemetry_bench_task, "dbench", 16384, nullptr, 24, nullptr, 1);
}

void loop()
{
    delay(1000);
}

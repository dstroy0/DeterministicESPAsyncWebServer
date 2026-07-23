// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// On-device CCOUNT microbenchmark for the dashboard widget-table JSON serializers
// (services/dashboard core): dws_dashboard_set() (the per-sample hot path that feeds
// telemetry into the widget table), the layout/values JSON serializers that back the
// page's initial fetch and each SSE publish, and the inbound control-message parser +
// dispatcher (WebSocket control messages from the page). All pure - no heap, no server,
// no SSE/WebSocket transport - so every call here exercises the real production code
// path. Worked example for perf/device/<service>/: a pure protocol codec with no
// hardware involved (contrast with perf/device/ads1115, a peripheral driver where the
// bus transaction itself is stubbed). dws_dashboard_begin()/dws_dashboard_publish()
// (server/SSE wiring in dashboard_routes.cpp) are deliberately out of scope, same split
// test_matrix.json draws for the host test suite: never called here.
//
// Build/flash (JTAG-capable S3 over its USB-Serial/JTAG port):
//   pio run -d perf/device/dashboard -t upload --upload-port COM7
// then open the port to capture the repeating "DB ..." lines (each run repeats every ~5 s, so a
// capture opened at any time still catches a full cycle).
#include "device_bench.h"
#include "services/dashboard/dashboard.h"
#include <Arduino.h>

// Two widgets (a scaled gauge + a plain value), the same shape as test/test_dashboard/test_dashboard.cpp.
static const DWSWidget kWidgets[] = {
    {DWSWidgetType::DWS_WIDGET_GAUGE, "Temp", "temp", 0, 100, "C"},
    {DWSWidgetType::DWS_WIDGET_VALUE, "Count", "count", 0, 0, ""},
};

// Satisfies dws_dashboard_on_control()'s callback requirement; dispatch never touches
// hardware, so a no-op is exactly what the host test uses too.
static void noop_control_cb(const char *, float)
{
}

static void dashboard_bench_task(void *)
{
    dws_dashboard_configure(kWidgets, 2);
    dws_dashboard_on_control(noop_control_cb);
    dws_dashboard_set("temp", 23.5f);
    dws_dashboard_set("count", 7.0f);

    static char layout_buf[512];
    static char values_buf[256];
    static char key_out[32];
    static const char kControlMsg[] = "{\"k\":\"temp\",\"v\":3.5}";
    float parsed_value = 0.0f;

    for (;;)
    {
        Serial.printf("DB ==== dashboard device microbench start (CCOUNT @ %u MHz) ====\n",
                      (unsigned)getCpuFrequencyMhz());
        volatile bool sinkb = false;
        volatile int sinki = 0;

        DBENCH_OP("dws_dashboard_set", 100000, sinkb = dws_dashboard_set("temp", 23.5f));
        DBENCH_OP("dws_dashboard_layout_json", 20000,
                  sinki += dws_dashboard_layout_json(layout_buf, sizeof(layout_buf)));
        DBENCH_OP("dws_dashboard_values_json", 50000,
                  sinki += dws_dashboard_values_json(values_buf, sizeof(values_buf)));
        DBENCH_OP("dws_dashboard_parse_control", 50000,
                  sinkb = dws_dashboard_parse_control(kControlMsg, key_out, sizeof(key_out), &parsed_value));
        DBENCH_OP("dws_dashboard_dispatch_control", 50000, sinkb = dws_dashboard_dispatch_control(kControlMsg));

        (void)sinkb;
        (void)sinki;
        Serial.println("DB ==== DONE ====");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2500); // let USB-CDC enumerate so the banner is captured
    Serial.println("\nDB boot: dashboard device microbench");
    xTaskCreatePinnedToCore(dashboard_bench_task, "dbench", 16384, nullptr, 24, nullptr, 1);
}

void loop()
{
    delay(1000);
}

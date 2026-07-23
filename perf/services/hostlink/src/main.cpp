// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// On-device CCOUNT microbenchmark for the Omron Host Link (C-mode) frame codec (services/hostlink):
// the FCS (an 8-bit XOR over the ASCII body), the command builder (dws_hostlink_build: @UU + header +
// text + FCS + *CR), the FCS-validating parser (dws_hostlink_parse), and the response end-code reader
// (dws_hostlink_end_code). Every operation here is pure - no heap, no sockets, no UART - so this is
// like perf/device/modbus, a pure protocol codec where each call exercises the real production code
// path. The RS-232/485 serial transport is the application's responsibility and is deliberately out
// of scope (nothing is wired to this rig); only the deterministic CPU-side framing is benched.
//
// Build/flash (JTAG-capable S3 over its USB-Serial/JTAG port):
//   pio run -d perf/device/hostlink -t upload --upload-port COM7
// then open the port to capture the repeating "DB ..." lines (each run repeats every ~5 s, so a
// capture opened at any time still catches a full cycle).
#include "device_bench.h"
#include "services/hostlink/hostlink.h"
#include <Arduino.h>

static void hostlink_bench_task(void *)
{
    // Known-good, spec-conformant vectors straight out of test/test_hostlink/test_hostlink.cpp.
    // Body "@00RD00000010" has FCS 0x57; the DM-read frame is "@00RD0000001057*\r" (17 chars).
    static const char body[] = "@00RD00000010";
    const size_t body_len = sizeof(body) - 1; // exclude the NUL

    static char frame[32];
    const size_t frame_len = dws_hostlink_build(frame, sizeof(frame), 0, "RD", "00000010", 8);

    static char outbuf[32];

    for (;;)
    {
        Serial.printf("DB ==== hostlink device microbench start (CCOUNT @ %u MHz) ====\n",
                      (unsigned)getCpuFrequencyMhz());
        volatile uint32_t sink = 0;

        // FCS: 8-bit XOR over the ASCII body (throughput over the frame body bytes).
        DBENCH_BULK("dws_hostlink_fcs", 100000, body_len, sink += dws_hostlink_fcs(body, body_len));

        // Build a full DM-read command frame (@UU + header + text + FCS + *CR).
        DBENCH_OP("dws_hostlink_build RD", 100000,
                  sink += dws_hostlink_build(outbuf, sizeof(outbuf), 0, "RD", "00000010", 8));

        // Parse + FCS-validate a complete frame.
        HostlinkFrame f;
        DBENCH_OP("dws_hostlink_parse", 100000, sink += dws_hostlink_parse(frame, frame_len, &f));

        // Read the response end code (first 2 text chars) off an already-parsed frame.
        (void)dws_hostlink_parse(frame, frame_len, &f);
        uint8_t code = 0;
        DBENCH_OP("dws_hostlink_end_code", 200000, sink += dws_hostlink_end_code(&f, &code));

        (void)sink;
        Serial.println("DB ==== DONE ====");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2500); // let USB-CDC enumerate so the banner is captured
    Serial.println("\nDB boot: hostlink device microbench");
    xTaskCreatePinnedToCore(hostlink_bench_task, "dbench", 16384, nullptr, 24, nullptr, 1);
}

void loop()
{
    delay(1000);
}

// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// On-device CCOUNT microbenchmark for the DMA peripheral ingest/egress simulator
// (services/dma): dws_dma_open/close (channel config + re-arm), dws_dma_sim_feed +
// dws_dma_poll (ingress -> ping-pong RX completion), dws_dma_tx_submit + dws_dma_poll +
// dws_dma_sim_capture (egress produce + read-back), and a loopback channel's TX->RX
// round trip - all pure, in-memory engine work (DWS_DMA_SIMULATE, the shipped/tested
// backend). Out of scope: dws_dma_hw_* - this rig has no physical UART/I2C/SPI loopback
// wire, so the real-silicon backend (DWS_DMA_SIMULATE=0) is never exercised here; the
// simulator IS the deterministic pipeline under test, identically to the host suite.
//
// Build/flash (JTAG-capable S3 over its USB-Serial/JTAG port):
//   pio run -d perf/device/dma -t upload --upload-port COM7
// then open the port to capture the repeating "DB ..." lines (each run repeats every ~5 s, so a
// capture opened at any time still catches a full cycle).
#include "device_bench.h"
#include "services/dma/dma.h"
#include <Arduino.h>

namespace
{
volatile uint32_t g_ev_bytes = 0;

void dma_bench_cb(const dws_dma_event *ev, void *ctx)
{
    (void)ctx;
    g_ev_bytes += ev->len;
}
} // namespace

static void dma_bench_task(void *)
{
    // Channel 0: plain UART channel, reopened inside the config bench and reused by the
    // ingress/egress benches. Channel 1: loopback (internal TX->RX jumper), opened once.
    dws_dma_config cfg0 = {};
    cfg0.channel = 0;
    cfg0.periph = dws_dma_periph::DWS_DMA_UART;
    cfg0.loopback = false;
    cfg0.on_complete = dma_bench_cb;
    cfg0.ctx = nullptr;

    dws_dma_config cfg1 = {};
    cfg1.channel = 1;
    cfg1.periph = dws_dma_periph::DWS_DMA_UART;
    cfg1.loopback = true;
    cfg1.on_complete = dma_bench_cb;
    cfg1.ctx = nullptr;

    dws_dma_open(&cfg0);
    dws_dma_open(&cfg1);

    // Exactly one DWS_DMA_BUF_SIZE buffer's worth of payload (drives one clean RX
    // completion + ping-pong flip per feed, no partial-flush remainder).
    static uint8_t payload[DWS_DMA_BUF_SIZE];
    for (size_t i = 0; i < sizeof(payload); i++)
        payload[i] = (uint8_t)(0x40 + i);
    static uint8_t capbuf[DWS_DMA_BUF_SIZE * 3];

    for (;;)
    {
        Serial.printf("DB ==== dma device microbench start (CCOUNT @ %u MHz) ====\n", (unsigned)getCpuFrequencyMhz());
        volatile size_t sink = 0;

        DBENCH_OP("dws_dma_close+open (ch reconfig)", 50000, (dws_dma_close(0), dws_dma_open(&cfg0)));

        DBENCH_BULK("dws_dma_sim_feed+poll (RX)", 30000, sizeof(payload),
                    (dws_dma_sim_feed(0, payload, (uint16_t)sizeof(payload)), dws_dma_poll()));

        DBENCH_BULK("dws_dma_tx_submit+poll+capture (TX)", 30000, sizeof(payload),
                    (dws_dma_tx_submit(0, payload, (uint16_t)sizeof(payload)), dws_dma_poll(),
                     sink += dws_dma_sim_capture(0, capbuf, sizeof(capbuf))));

        DBENCH_BULK("dws_dma_loopback_round_trip (TX+RX)", 20000, sizeof(payload),
                    (dws_dma_tx_submit(1, payload, (uint16_t)sizeof(payload)), dws_dma_poll()));

        (void)sink;
        Serial.println("DB ==== DONE ====");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2500); // let USB-CDC enumerate so the banner is captured
    Serial.println("\nDB boot: dma device microbench");
    xTaskCreatePinnedToCore(dma_bench_task, "dbench", 16384, nullptr, 24, nullptr, 1);
}

void loop()
{
    delay(1000);
}

// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// On-device CCOUNT microbenchmark for the SNMP agent (services/snmp): the v2c GET and GETNEXT
// request processing (BER decode, MIB dispatch, response BER encode) - the untrusted-input hot op an
// SNMP agent runs per datagram. The request datagrams are built in-buffer with the BER encoder; the
// UDP socket is out of scope.
//
// Build/flash:  pio run -d perf/device/snmp -t upload --upload-port COM7
#include "device_bench.h"
#include "services/snmp/snmp_agent.h"
#include "services/snmp/snmp_ber.h"
#include <Arduino.h>

// Build a v2c request datagram (mirrors the SNMP agent test's builder + the host bench).
static size_t build_req(uint8_t *buf, size_t cap, uint8_t pdu, long reqid, const uint32_t *oid, size_t oidn)
{
    BerEnc e;
    dws_ber_enc_init(&e, buf, cap);
    size_t msg = dws_ber_seq_begin(&e, (uint8_t)SnmpTag::BER_SEQUENCE);
    dws_ber_put_integer(&e, 1); // v2c
    dws_ber_put_octet_string(&e, (uint8_t)SnmpTag::BER_OCTET_STRING, (const uint8_t *)"public", 6);
    size_t pdus = dws_ber_seq_begin(&e, pdu);
    dws_ber_put_integer(&e, reqid);
    dws_ber_put_integer(&e, 0);
    dws_ber_put_integer(&e, 0);
    size_t vbl = dws_ber_seq_begin(&e, (uint8_t)SnmpTag::BER_SEQUENCE);
    size_t vb = dws_ber_seq_begin(&e, (uint8_t)SnmpTag::BER_SEQUENCE);
    dws_ber_put_oid(&e, oid, oidn);
    dws_ber_put_null(&e);
    dws_ber_seq_end(&e, vb);
    dws_ber_seq_end(&e, vbl);
    dws_ber_seq_end(&e, pdus);
    dws_ber_seq_end(&e, msg);
    return e.ok ? e.len : 0;
}

static void snmp_bench_task(void *)
{
    dws_snmp_agent_init("public");
    dws_snmp_agent_set_system("DeterministicESPAsyncWebServer SNMP agent bench", "admin@dws", "esp32-dws", "lab", 72);

    static const uint32_t OID_SYSDESCR[] = {1, 3, 6, 1, 2, 1, 1, 1, 0}; // sysDescr.0
    static const uint32_t OID_SYSTEM[] = {1, 3, 6, 1, 2, 1, 1};         // system group (GetNext root)
    static uint8_t reqget[128], reqnext[128], resp[512];
    size_t nget = build_req(reqget, sizeof(reqget), (uint8_t)SnmpTag::SNMP_PDU_GET, 0x0102, OID_SYSDESCR, 9);
    size_t nnext = build_req(reqnext, sizeof(reqnext), (uint8_t)SnmpTag::SNMP_PDU_GETNEXT, 0x0103, OID_SYSTEM, 7);

    for (;;)
    {
        Serial.printf("DB ==== snmp device microbench start (CCOUNT @ %u MHz) ====\n", (unsigned)getCpuFrequencyMhz());
        volatile size_t sink = 0;
        DBENCH_OP("dws_snmp_agent_process GET", 100000,
                  sink += dws_snmp_agent_process(reqget, nget, resp, sizeof(resp)));
        DBENCH_OP("dws_snmp_agent_process GETNEXT", 100000,
                  sink += dws_snmp_agent_process(reqnext, nnext, resp, sizeof(resp)));
        (void)sink;
        Serial.println("DB ==== DONE ====");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2500);
    Serial.println("\nDB boot: snmp device microbench");
    xTaskCreatePinnedToCore(snmp_bench_task, "dbench", 16384, nullptr, 24, nullptr, 1);
}

void loop()
{
    delay(1000);
}

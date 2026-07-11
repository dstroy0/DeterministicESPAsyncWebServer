// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host-side microbenchmark for the RFC 5424 syslog client formatter: syslog_format() builds one
// `<PRI>1 - HOST APP - - - MSG` line into a caller buffer - the per-log-line hot op the device runs before
// each det_udp_sendto(). Pure (no socket, no heap). syslog.cpp also holds syslog_log() which references
// det_udp_sendto(), so link udp.cpp for the host no-op UDP stubs (same pattern as bench_coap/bench_snmp).
// The device number comes from the rig /bench syslog_format op; this host ns/op is a RELATIVE baseline:
//   g++ -O2 -std=c++17 -Isrc -Itest/mocks -Itest/support -DDETWS_ENABLE_SYSLOG=1 perf/bench_syslog.cpp \
//       src/services/syslog/syslog.cpp src/network_drivers/transport/udp.cpp -o /tmp/bsl && /tmp/bsl

#define DETWS_ENABLE_SYSLOG 1
#include "services/syslog/syslog.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>

using clk = std::chrono::steady_clock;

template <typename F> static double bench_ns(uint64_t iters, F fn)
{
    auto t0 = clk::now();
    for (uint64_t i = 0; i < iters; i++)
        fn();
    auto t1 = clk::now();
    double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
    return ns / (double)iters;
}

static void row(const char *feature, const char *op, double ns_per_op, double bytes_per_op)
{
    double mbps = bytes_per_op > 0 ? (bytes_per_op / (ns_per_op * 1e-9)) / 1e6 : 0.0;
    printf("| %-12s | %-24s | %10.1f | %9.1f |\n", feature, op, ns_per_op, mbps);
}

int main()
{
    char out[256];
    const char *msg = "sensor=21.4C rh=48% link=up heap=131072";
    size_t len = syslog_format(out, sizeof(out), SyslogFacility::SYSLOG_FAC_LOCAL0, SyslogSeverity::SYSLOG_INFO,
                               "detws-rig", "rig-app", msg);

    printf("| Feature      | Operation                |     ns/op |    MB/s |\n");
    printf("|--------------|--------------------------|-----------|---------|\n");

    // format one RFC 5424 line (the per-log-line cost before the UDP send).
    {
        volatile size_t sink = 0;
        double ns = bench_ns(2000000, [&] {
            sink += syslog_format(out, sizeof(out), SyslogFacility::SYSLOG_FAC_LOCAL0, SyslogSeverity::SYSLOG_INFO,
                                  "detws-rig", "rig-app", msg);
        });
        row("syslog", "syslog_format (RFC 5424)", ns, (double)len);
        (void)sink;
    }

    return 0;
}

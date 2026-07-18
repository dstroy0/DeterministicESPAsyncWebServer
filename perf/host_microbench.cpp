// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host-side microbenchmarks for pure codecs: a deterministic ns/op + MB/s baseline that
// complements the on-device ESP32-S3 numbers in docs/FEATURE_PERFORMANCE.md. The host figure
// is a RELATIVE baseline (a fast desktop/RPi core), not the real-world device cost; it exists
// to compare features against each other and to catch regressions cheaply. Build + run:
//   g++ -O2 -std=c++17 -Isrc -DDETWS_ENABLE_MTCONNECT=1 perf/host_microbench.cpp \
//       src/network_drivers/presentation/base64/base64.cpp \
//       src/services/mtconnect/mtconnect.cpp -o /tmp/hb && /tmp/hb

#include "services/mtconnect/mtconnect.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "network_drivers/presentation/base64/base64.h"

using clk = std::chrono::steady_clock;

// Time `iters` calls of fn(), return ns per op.
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
    if (bytes_per_op > 0)
        printf("| %-12s | %-22s | %10.1f | %9.1f |\n", feature, op, ns_per_op, mbps);
    else
        printf("| %-12s | %-22s | %10.1f | %9s |\n", feature, op, ns_per_op, "-");
}

int main()
{
    printf("| Feature      | Operation              |     ns/op |    MB/s |\n");
    printf("|--------------|------------------------|-----------|---------|\n");

    // --- base64: encode + decode a 1 KiB payload ---
    {
        const size_t N = 1024;
        uint8_t src[N];
        for (size_t i = 0; i < N; i++)
            src[i] = (uint8_t)(i * 31 + 7);
        char enc[((N + 2) / 3) * 4 + 1];
        uint8_t dec[N];
        volatile size_t sink = 0;
        double ns_e = bench_ns(200000, [&] { base64_encode(src, N, enc); });
        row("base64", "encode 1 KiB", ns_e, (double)N);
        double ns_d = bench_ns(200000, [&] { sink += base64_decode(enc, dec, sizeof(dec)); });
        row("base64", "decode 1 KiB", ns_d, (double)N);
        (void)sink;
    }

    // --- MTConnect: build a streams document of 20 observations ---
    {
        char buf[4096];
        volatile size_t sink = 0;
        double ns = bench_ns(200000, [&] {
            DWSMtcStreams s;
            detws_mtc_streams_begin(&s, buf, sizeof(buf), 1500, 20, "cnc1");
            for (int i = 0; i < 20; i++)
                detws_mtc_streams_add(&s, DETWS_MTC_SAMPLE, "Position", "xpos", (uint64_t)i, "2026-07-09T00:00:00Z",
                                      "12.5");
            sink += detws_mtc_streams_end(&s);
        });
        row("mtconnect", "streams doc (20 obs)", ns, (double)sink / 200000.0);
        (void)sink;
    }

    return 0;
}

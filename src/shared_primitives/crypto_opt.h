// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file crypto_opt.h
 * @brief Per-translation-unit optimization override for hot, pure-integer crypto.
 *
 * The library ships at the arduino framework's `-Os` (the framework appends it AFTER any PlatformIO env
 * `build_flags`, so a plain `-O2` there is overridden). `-Os` roughly halves the throughput of the software
 * ciphers/MACs. Put @ref DWS_CRYPTO_HOT at the top of such a `.cpp` (after its includes) to force a higher
 * level for that one translation unit, regardless of the consumer's size-optimized build:
 *
 * @code
 *   #include "ssh_chacha20.h"
 *   #include "shared_primitives/crypto_opt.h"
 *   DWS_CRYPTO_HOT   // this TU builds at -O2 (or the configured level)
 * @endcode
 *
 * Configure with `DWS_CRYPTO_OPT_LEVEL` (define in `build_flags` or ServerConfig): `2` (default) or `3`;
 * define it to `0` to inherit the framework `-Os`. Only GCC honors `#pragma GCC optimize`; clang/other
 * compilers get a no-op and inherit their normal level.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * CAVEATS - read before applying this to a new file
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. CONSTANT TIME. Apply this ONLY to code that is constant-time by STRUCTURE - no secret-dependent
 *    branches and no secret-dependent memory indexing: stream ciphers (ChaCha20), MACs (Poly1305), hashes.
 *    Do NOT put it on scalar-multiplication / bignum / point-arithmetic code that relies on branchless
 *    mask-selects to stay constant-time: an aggressive optimizer can (rarely, but it is documented) turn a
 *    mask-select into a data-dependent branch and reintroduce a timing side channel. Those paths are also
 *    HW-accelerator-dominated (RSA/MPI MODMULT, HW AES/SHA), so the `-O` level buys them almost nothing -
 *    all risk, no reward. When in doubt, leave it off.
 *
 * 2. SIZE. `-O2`/`-O3` inline and unroll aggressively -> larger flash and IRAM; `-O3` more so. On the
 *    classic ESP32 (tight internal DRAM/IRAM, where TLS example builds already run close to the limit)
 *    prefer level `2` or `0`.
 *
 * 3. `-O3` IS NOT RELIABLY FASTER than `-O2` here. The bigger unrolled code can thrash the instruction /
 *    flash cache and regress, and `-O3` widens the miscompile and latent-UB surface. `-O2` captures
 *    essentially all of the practical win; treat `3` as a measure-it-yourself option, not a default.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CRYPTO_OPT_H
#define DETERMINISTICESPASYNCWEBSERVER_CRYPTO_OPT_H

#if defined(__GNUC__) && !defined(__clang__)
#ifndef DWS_CRYPTO_OPT_LEVEL
#define DWS_CRYPTO_OPT_LEVEL 2 // default: -O2 (the -Os -> -O2 jump is the ~2x win; O2 -> O3 is marginal)
#endif
#if DWS_CRYPTO_OPT_LEVEL == 3
#define DWS_CRYPTO_HOT _Pragma("GCC optimize(\"O3\")")
#elif DWS_CRYPTO_OPT_LEVEL == 2
#define DWS_CRYPTO_HOT _Pragma("GCC optimize(\"O2\")")
#else
#define DWS_CRYPTO_HOT // 0 / other: inherit the framework -Os
#endif
#else
#define DWS_CRYPTO_HOT // non-GCC: no per-TU pragma; inherit the toolchain default
#endif

#endif // DETERMINISTICESPASYNCWEBSERVER_CRYPTO_OPT_H

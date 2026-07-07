// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file exc_decoder.h
 * @brief ESP32 panic / exception decoder for a live diagnostics panel (DETWS_ENABLE_EXC_DECODER).
 *
 * When an ESP32 panics it prints a Guru Meditation dump: a cause ("LoadProhibited"), a per-core register
 * dump (PC, EXCVADDR, ...), and a backtrace of `PC:SP` frame pairs. To resolve those PCs to file:line an
 * addr2line-style panel needs the firmware ELF, which lives off-device - so the on-device job is to
 * *extract and present* the raw decode: the cause, the faulting PC + data address, and the ordered
 * backtrace PC list, served as JSON for a live "/exception" panel (the browser or a build server then
 * resolves symbols). This is that extractor.
 *
 * It parses the panic text an app captures (from the console, a saved crash line, or a text rendering of
 * the core-dump partition) into a structured ExcInfo, and serializes it. Pure, zero heap, no stdlib
 * (hand-rolled hex/decimal parsing), host-testable against a captured panic string.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_EXC_DECODER_H
#define DETERMINISTICESPASYNCWEBSERVER_EXC_DECODER_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_EXC_DECODER

#ifndef DETWS_EXC_MAX_FRAMES
#define DETWS_EXC_MAX_FRAMES 32 ///< backtrace frames retained (Xtensa panics rarely exceed this).
#endif

/** @brief One backtrace frame: a program counter and its stack pointer. */
struct ExcFrame
{
    uint32_t pc;
    uint32_t sp;
};

/** @brief A decoded panic. Fields not found in the input are left at their zeroed / -1 defaults. */
struct ExcInfo
{
    int core;                              ///< panicking core number, or -1 if not present.
    char cause[32];                        ///< exception cause text (e.g. "LoadProhibited"), "" if absent.
    uint32_t pc;                           ///< faulting PC (register-dump PC, else first backtrace frame).
    uint32_t excvaddr;                     ///< faulting data address (EXCVADDR), 0 if absent.
    bool has_excvaddr;                     ///< true if an EXCVADDR field was present.
    ExcFrame frames[DETWS_EXC_MAX_FRAMES]; ///< backtrace, outermost-first as printed.
    size_t frame_count;
};

/**
 * @brief Parse an ESP32 panic dump into @p out.
 *
 * Recognizes the Guru Meditation cause, the core number, the register-dump PC + EXCVADDR, and the
 * `Backtrace: pc:sp pc:sp ...` frame list. Tolerant of missing fields and of a trailing "|<-CORRUPTED".
 * @return true if at least one of {cause, pc, a backtrace frame} was found.
 */
bool detws_exc_parse(const char *text, ExcInfo *out);

/**
 * @brief Serialize a decoded panic as
 * `{"core":N,"cause":"..","pc":"0x..","excvaddr":"0x..","backtrace":["0x..",...]}`.
 * `core` is omitted when -1; `excvaddr` is omitted when absent.
 * @return length written (excl NUL), or 0 on overflow / bad args.
 */
size_t detws_exc_json(const ExcInfo *info, char *out, size_t cap);

#endif // DETWS_ENABLE_EXC_DECODER
#endif // DETERMINISTICESPASYNCWEBSERVER_EXC_DECODER_H

// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file psram_pool.h
 * @brief Buffer placement policy (DRAM vs PSRAM) + SPI DMA ping-pong index manager
 *        (DETWS_ENABLE_PSRAM_POOL).
 *
 * An ESP32 with PSRAM has two heaps: fast internal DRAM (scarce, DMA-capable) and large external PSRAM
 * (roomy, but not DMA-capable for most peripherals and slower). Serving big web assets / net buffers well
 * means putting the large, cold buffers in PSRAM and keeping the small, hot, DMA buffers in DRAM, while
 * always leaving an internal-DRAM reserve so the stack does not starve. That placement choice is a pure
 * policy; the actual `heap_caps_calloc(..., MALLOC_CAP_SPIRAM / MALLOC_CAP_DMA)` is the app's.
 *
 * This module is that policy (`detws_psram_place`) plus the classic SPI DMA **ping-pong** double-buffer
 * bookkeeping (`detws_pingpong_*`): while DMA drains one buffer, the CPU fills the other, and a swap
 * exchanges their roles. Pure, no heap, no stdlib, host-testable.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_PSRAM_POOL_H
#define DETERMINISTICESPASYNCWEBSERVER_PSRAM_POOL_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_PSRAM_POOL

/** @brief Placement verdict. */
enum
{
    PLACE_DRAM = 0,  ///< allocate in internal DRAM.
    PLACE_PSRAM = 1, ///< allocate in external PSRAM.
    PLACE_FAIL = 2   ///< neither heap can satisfy the request.
};

/**
 * @brief Decide where a buffer should live.
 *
 * Rules: a zero-size request fails; a DMA-required buffer must go in DRAM (or fail); a buffer at or above
 * @p psram_threshold prefers PSRAM (falling back to DRAM); a smaller buffer prefers DRAM (falling back to
 * PSRAM). A DRAM placement must still leave @p dram_reserve bytes free in DRAM.
 *
 * @param size           requested bytes.
 * @param dma_required   true if the buffer must be DMA-capable (DRAM only).
 * @param free_dram      currently free internal DRAM.
 * @param free_psram     currently free PSRAM (0 if no PSRAM).
 * @param psram_threshold size at/above which PSRAM is preferred.
 * @param dram_reserve   internal DRAM to keep free after a DRAM placement.
 * @return PLACE_DRAM / PLACE_PSRAM / PLACE_FAIL.
 */
int detws_psram_place(size_t size, bool dma_required, size_t free_dram, size_t free_psram, size_t psram_threshold,
                      size_t dram_reserve);

/** @brief SPI DMA ping-pong double-buffer state. */
struct PingPong
{
    uint8_t fill_idx; ///< buffer the CPU is filling (DMA drains the other).
};

/** @brief Initialize: CPU fills buffer 0, DMA drains buffer 1. */
void detws_pingpong_init(PingPong *pp);

/** @brief The buffer index the CPU should fill. */
uint8_t detws_pingpong_fill_index(const PingPong *pp);

/** @brief The buffer index DMA should drain (the other one). */
uint8_t detws_pingpong_drain_index(const PingPong *pp);

/** @brief Swap roles (a filled buffer is handed to DMA; the drained one is now filled). @return new fill index. */
uint8_t detws_pingpong_swap(PingPong *pp);

#endif // DETWS_ENABLE_PSRAM_POOL
#endif // DETERMINISTICESPASYNCWEBSERVER_PSRAM_POOL_H

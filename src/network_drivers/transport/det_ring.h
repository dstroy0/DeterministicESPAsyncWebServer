// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef DETERMINISTICESPASYNCWEBSERVER_DET_RING_H
#define DETERMINISTICESPASYNCWEBSERVER_DET_RING_H

/**
 * @file det_ring.h
 * @brief Shared single-producer / single-consumer byte-ring primitive.
 *
 * The one implementation of the receive-ring drain math, used by BOTH transports:
 * the server (det_conn_* in transport.h, over conn_pool slots) and the outbound
 * client (det_client_* over its pool). Sharing it is deliberate - it keeps the ring
 * invariants in a single place so a consumer in any layer drains identically and
 * cannot reimplement (and subtly break) the wrap/ordering by hand.
 *
 * Ownership rule: exactly one producer advances `head`, exactly one consumer
 * advances `tail`; indices are DetAtomic so a producer's buffer writes are visible
 * before the consumer observes the advanced index (acquire/release), correct across
 * the tcpip_thread <-> worker/caller boundary on either core. No locks, no RMW.
 */

#include <atomic>
#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Cross-thread field wrapper
// ---------------------------------------------------------------------------
//
// Fields shared across a producer/consumer thread boundary (ring head/tail, slot
// state) are wrapped in DetAtomic so the ordering is enforced by construction
// rather than by convention: every read is an acquire load and every write a
// release store, so a producer's buffer writes are visible before the consumer
// observes the advanced index (single-producer / single-consumer, so no
// read-modify-write atomicity is needed - only ordering). On Xtensa/x86 an aligned
// acquire/release load/store is a plain load/store plus a compiler barrier, so this
// adds no lock and no measurable cost - determinism (bounded latency) is preserved.
// The conversion operators keep every ==/=/arithmetic call site unchanged; the copy
// ctor/assignment let a containing aggregate still be value-reset (`x = {}`).
template <typename T> struct DetAtomic
{
    std::atomic<T> v;
    DetAtomic() noexcept : v(T())
    {
    }
    DetAtomic(T x) noexcept : v(x)
    {
    }
    DetAtomic(const DetAtomic &o) noexcept : v(o.v.load(std::memory_order_acquire))
    {
    }
    DetAtomic &operator=(const DetAtomic &o) noexcept
    {
        v.store(o.v.load(std::memory_order_acquire), std::memory_order_release);
        return *this;
    }
    DetAtomic &operator=(T x) noexcept
    {
        v.store(x, std::memory_order_release);
        return *this;
    }
    operator T() const noexcept
    {
        return v.load(std::memory_order_acquire);
    }
};

// ---------------------------------------------------------------------------
// SPSC ring drain math (consumer side)
// ---------------------------------------------------------------------------
// The caller owns the storage (`buf` of `cap` bytes) and the indices; these advance
// `tail` only (the producer owns `head`). A read reads `tail` once and publishes it
// once at the end (one release store), not per byte.

/** @brief Bytes available to read (head - tail, modulo cap). */
static inline size_t det_ring_available(const DetAtomic<size_t> &head, const DetAtomic<size_t> &tail, size_t cap)
{
    return ((size_t)head + cap - (size_t)tail) % cap;
}

/** @brief Pop one byte into @p out; false if empty. */
static inline bool det_ring_read_byte(const uint8_t *buf, size_t cap, const DetAtomic<size_t> &head,
                                      DetAtomic<size_t> &tail, uint8_t *out)
{
    size_t t = tail;
    if (t == (size_t)head)
        return false;
    *out = buf[t];
    tail = (t + 1) % cap;
    return true;
}

/** @brief Pop up to @p maxn bytes into @p dst; returns the count read. */
static inline size_t det_ring_read(const uint8_t *buf, size_t cap, const DetAtomic<size_t> &head,
                                   DetAtomic<size_t> &tail, uint8_t *dst, size_t maxn)
{
    size_t h = head, t = tail, n = 0;
    while (n < maxn && t != h)
    {
        dst[n++] = buf[t];
        t = (t + 1) % cap;
    }
    tail = t;
    return n;
}

/** @brief Copy @p n bytes at @p off ahead of the tail into @p dst WITHOUT consuming. */
static inline void det_ring_peek(const uint8_t *buf, size_t cap, const DetAtomic<size_t> &tail, size_t off,
                                 uint8_t *dst, size_t n)
{
    size_t idx = ((size_t)tail + off) % cap;
    for (size_t i = 0; i < n; i++)
    {
        dst[i] = buf[idx];
        idx = (idx + 1) % cap;
    }
}

/** @brief Drop @p n bytes from the tail (advance past already-peeked data). */
static inline void det_ring_consume(DetAtomic<size_t> &tail, size_t cap, size_t n)
{
    tail = ((size_t)tail + n) % cap;
}

#endif // DETERMINISTICESPASYNCWEBSERVER_DET_RING_H

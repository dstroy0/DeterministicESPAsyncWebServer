// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file auth_lockout.cpp
 * @brief Per-IP brute-force lockout state machine (DETWS_ENABLE_AUTH_LOCKOUT).
 *
 * A bounded BSS table of buckets keyed by source IPv4. Each bucket holds the
 * consecutive-failure count and, once the threshold is crossed, the start and
 * duration of the current lockout (exponential backoff, capped). Compiled only
 * when DETWS_ENABLE_AUTH_LOCKOUT is set; the host unit tests enable it.
 */

#include "auth_lockout.h"

#if DETWS_ENABLE_AUTH_LOCKOUT

namespace
{

struct LockoutBucket
{
    uint32_t ip;            ///< source IPv4 key; 0 marks an empty bucket.
    uint32_t lock_start_ms; ///< millis() when the current lockout began.
    uint32_t lock_ms;       ///< current lockout duration (0 = not locked).
    uint32_t last_ms;       ///< millis() of the last recorded failure (LRU eviction).
    uint16_t fails;         ///< consecutive failures from this address.
};

LockoutBucket g_buckets[DETWS_AUTH_LOCKOUT_SLOTS];

LockoutBucket *find_bucket(uint32_t ip)
{
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_SLOTS; i++)
        if (g_buckets[i].ip == ip)
            return &g_buckets[i];
    return nullptr;
}

bool bucket_locked(const LockoutBucket *b, uint32_t now_ms)
{
    // Unsigned subtraction wraps correctly across the millis() rollover.
    return b->lock_ms != 0 && (uint32_t)(now_ms - b->lock_start_ms) < b->lock_ms;
}

} // namespace

uint32_t auth_lockout_remaining_ms(uint32_t ip, uint32_t now_ms)
{
    if (ip == 0)
        return 0; // untrackable source -> never reported as locked
    LockoutBucket *b = find_bucket(ip);
    if (!b || b->lock_ms == 0)
        return 0;
    uint32_t elapsed = now_ms - b->lock_start_ms; // wraps correctly across rollover
    if (elapsed >= b->lock_ms)
        return 0; // the lockout window has passed
    return b->lock_ms - elapsed;
}

void auth_lockout_fail(uint32_t ip, uint32_t now_ms)
{
    if (ip == 0)
        return; // untrackable source

    LockoutBucket *b = find_bucket(ip);
    if (!b)
    {
        // Claim a bucket: an empty one first; else evict the least-recently-used
        // address that is NOT currently locked; only if every bucket is locked do
        // we evict the overall LRU (so an active attacker cannot evict their own
        // lockout by flooding from other addresses).
        int slot = -1, lru = 0;
        for (int i = 0; i < DETWS_AUTH_LOCKOUT_SLOTS; i++)
        {
            if (g_buckets[i].ip == 0)
            {
                slot = i;
                break;
            }
            if ((uint32_t)(now_ms - g_buckets[i].last_ms) > (uint32_t)(now_ms - g_buckets[lru].last_ms))
                lru = i;
            if (!bucket_locked(&g_buckets[i], now_ms) &&
                (slot < 0 || (uint32_t)(now_ms - g_buckets[i].last_ms) > (uint32_t)(now_ms - g_buckets[slot].last_ms)))
                slot = i;
        }
        if (slot < 0)
            slot = lru; // table full of active lockouts
        b = &g_buckets[slot];
        b->ip = ip;
        b->fails = 0;
        b->lock_ms = 0;
        b->lock_start_ms = now_ms;
    }

    b->last_ms = now_ms;
    if (b->fails < 0xFFFF)
        b->fails++;

    if (b->fails >= DETWS_AUTH_LOCKOUT_THRESHOLD)
    {
        // Exponential backoff: base << (fails - threshold), capped at max. Double
        // by stepping so the shift can never overflow.
        uint32_t dur = DETWS_AUTH_LOCKOUT_BASE_MS;
        for (uint16_t n = (uint16_t)(b->fails - DETWS_AUTH_LOCKOUT_THRESHOLD); n > 0; n--)
        {
            if (dur >= DETWS_AUTH_LOCKOUT_MAX_MS || dur > (0xFFFFFFFFu >> 1))
            {
                dur = DETWS_AUTH_LOCKOUT_MAX_MS;
                break;
            }
            dur <<= 1;
        }
        if (dur > DETWS_AUTH_LOCKOUT_MAX_MS)
            dur = DETWS_AUTH_LOCKOUT_MAX_MS;
        b->lock_ms = dur;
        b->lock_start_ms = now_ms;
    }
}

void auth_lockout_succeed(uint32_t ip)
{
    if (ip == 0)
        return;
    LockoutBucket *b = find_bucket(ip);
    if (b)
    {
        b->ip = 0;
        b->fails = 0;
        b->lock_ms = 0;
        b->lock_start_ms = 0;
        b->last_ms = 0;
    }
}

void auth_lockout_reset(void)
{
    for (int i = 0; i < DETWS_AUTH_LOCKOUT_SLOTS; i++)
    {
        g_buckets[i].ip = 0;
        g_buckets[i].lock_start_ms = 0;
        g_buckets[i].lock_ms = 0;
        g_buckets[i].last_ms = 0;
        g_buckets[i].fails = 0;
    }
}

#endif // DETWS_ENABLE_AUTH_LOCKOUT

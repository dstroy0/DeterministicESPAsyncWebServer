// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit and stress tests for Layer 4 (Transport) - constants, pool invariants,
// ring-buffer arithmetic, timeout logic, event-queue behavior, and
// sustained-load correctness.

#include "network_drivers/network/ip.h"
#include "network_drivers/transport/listener.h"
#include "network_drivers/transport/tcp.h"
#include <unity.h>

// tcp.cpp + listener.cpp are compiled into the native env - no stubs needed.

// Build a v4 DetIp from a host-order word (0xC0A80005 -> 192.168.0.5). The accept-time
// gates key on the full family-tagged address, so the tests carry a DetIp, not a uint32.
static DetIp v4w(uint32_t host_order)
{
    return det_ip_from_v4_octets((uint8_t)(host_order >> 24), (uint8_t)(host_order >> 16), (uint8_t)(host_order >> 8),
                                 (uint8_t)host_order);
}

void setUp()
{
    set_millis(0);
    DeterministicAsyncTCP::pool_init();
    listener_add(0, 80, ConnProto::PROTO_HTTP);
}

void tearDown()
{
}

// ====================================================================
// UNIT TESTS
// ====================================================================

// ---- Compile-time constants ----------------------------------------

void test_pool_capacity_is_four()
{
    TEST_ASSERT_EQUAL(4, MAX_CONNS);
}
void test_rx_buffer_size_is_one_kb()
{
    TEST_ASSERT_EQUAL(1024, RX_BUF_SIZE);
}
void test_timeout_constant_is_5000ms()
{
    TEST_ASSERT_EQUAL(5000, CONN_TIMEOUT_MS);
}

// ---- Pool defaults after init() ------------------------------------

void test_all_slots_free_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[i].state);
}

void test_all_pcbs_null_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_NULL(conn_pool[i].pcb);
}

void test_all_ring_buffers_empty_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(conn_pool[i].rx_head, conn_pool[i].rx_tail);
}

void test_slot_ids_match_indices()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(i, conn_pool[i].id);
}

// ---- Ring buffer arithmetic ----------------------------------------

void test_ring_empty_when_head_equals_tail()
{
    TcpConn s = {};
    TEST_ASSERT_EQUAL(s.rx_head, s.rx_tail);
}

void test_ring_wrap_at_boundary()
{
    size_t next = (size_t)(RX_BUF_SIZE - 1 + 1) % RX_BUF_SIZE;
    TEST_ASSERT_EQUAL(0, (int)next);
}

void test_ring_full_sentinel_one_slot_reserved()
{
    size_t tail = 0;
    size_t head = RX_BUF_SIZE - 1;
    TEST_ASSERT_EQUAL(tail, (head + 1) % RX_BUF_SIZE);
}

void test_ring_can_store_size_minus_one_bytes()
{
    TcpConn s = {};
    s.rx_head = 0;
    s.rx_tail = 0;
    size_t count = 0;
    while (true)
    {
        size_t next = (s.rx_head + 1) % RX_BUF_SIZE;
        if (next == s.rx_tail)
            break;
        s.rx_buffer[s.rx_head] = (uint8_t)count;
        s.rx_head = next;
        count++;
    }
    TEST_ASSERT_EQUAL(RX_BUF_SIZE - 1, (int)count);
}

// ---- Event types ---------------------------------------------------

void test_event_types_are_distinct()
{
    TEST_ASSERT_NOT_EQUAL((int)EvtType::EVT_CONNECT, (int)EvtType::EVT_DATA);
    TEST_ASSERT_NOT_EQUAL((int)EvtType::EVT_DATA, (int)EvtType::EVT_DISCONNECT);
    TEST_ASSERT_NOT_EQUAL((int)EvtType::EVT_DISCONNECT, (int)EvtType::EVT_ERROR);
    TEST_ASSERT_NOT_EQUAL((int)EvtType::EVT_CONNECT, (int)EvtType::EVT_ERROR);
}

// ---- Timeout logic -------------------------------------------------

void test_timeout_does_not_fire_on_free_slot()
{
    conn_pool[0].state = ConnState::CONN_FREE;
    set_millis(CONN_TIMEOUT_MS + 1);
    DeterministicAsyncTCP::check_timeouts();
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
}

void test_timeout_does_not_fire_before_deadline()
{
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].last_activity_ms = 0;
    set_millis(CONN_TIMEOUT_MS - 1);
    DeterministicAsyncTCP::check_timeouts();
    TEST_ASSERT_EQUAL(ConnState::CONN_ACTIVE, (ConnState)conn_pool[0].state);
}

void test_timeout_fires_at_deadline()
{
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].last_activity_ms = 0;
    set_millis(CONN_TIMEOUT_MS);
    DeterministicAsyncTCP::check_timeouts();
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_NULL(conn_pool[0].pcb);
}

void test_timeout_fires_only_on_stale_slots()
{
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].pcb = nullptr;
    conn_pool[0].last_activity_ms = 0;

    conn_pool[1].state = ConnState::CONN_ACTIVE;
    conn_pool[1].pcb = nullptr;
    conn_pool[1].last_activity_ms = CONN_TIMEOUT_MS; // fresh

    set_millis(CONN_TIMEOUT_MS);
    DeterministicAsyncTCP::check_timeouts();

    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_EQUAL(ConnState::CONN_ACTIVE, (ConnState)conn_pool[1].state);
}

void test_init_succeeds_on_native()
{
    DeterministicAsyncTCP::pool_init();
    int32_t ok = listener_add(0, 80, ConnProto::PROTO_HTTP);
    TEST_ASSERT_EQUAL(1, ok);
}

// Regression: init() must zero last_activity_ms (was left uninitialized
// before the conn_pool[i]={} fix, causing cross-test timeout spurious fires).
void test_all_last_activity_ms_zero_after_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        TEST_ASSERT_EQUAL(0, (int)conn_pool[i].last_activity_ms);
}

void test_queue_not_null_after_init()
{
    TEST_ASSERT_NOT_NULL(listener_pool[0].queue);
}

// ====================================================================
// STRESS TESTS
// ====================================================================

// Fill the ring buffer to max capacity with a known pattern,
// then drain and verify every byte - no corruption under full load.
void stress_ring_buffer_fill_drain_integrity()
{
    TcpConn *s = &conn_pool[0];
    s->rx_head = 0;
    s->rx_tail = 0;
    const int FILL = RX_BUF_SIZE - 1; // sentinel leaves one slot empty

    // Write known pattern
    for (int i = 0; i < FILL; i++)
    {
        size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
        s->rx_buffer[s->rx_head] = (uint8_t)(i & 0xFF);
        s->rx_head = next;
    }

    // Buffer must report full (next_head == tail)
    TEST_ASSERT_EQUAL(RX_BUF_SIZE - 1, (int)((s->rx_head - s->rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE));

    // Drain and verify every byte
    for (int i = 0; i < FILL; i++)
    {
        uint8_t expected = (uint8_t)(i & 0xFF);
        uint8_t actual = s->rx_buffer[s->rx_tail];
        s->rx_tail = (s->rx_tail + 1) % RX_BUF_SIZE;
        TEST_ASSERT_EQUAL_MESSAGE(expected, actual, "ring buffer byte mismatch");
    }

    // Must be empty
    TEST_ASSERT_EQUAL(s->rx_head, s->rx_tail);
}

// Half-fill, half-drain, repeat - forces pointer wrap-around multiple
// times and proves the circular invariant holds under sustained partial load.
void stress_ring_buffer_multi_cycle_no_corruption()
{
    TcpConn *s = &conn_pool[0];
    s->rx_head = 0;
    s->rx_tail = 0;

    uint8_t write_val = 0;
    uint8_t read_val = 0;

    for (int cycle = 0; cycle < 8; cycle++)
    {
        const int BATCH = RX_BUF_SIZE / 2; // 512 bytes per batch

        for (int i = 0; i < BATCH; i++)
        {
            size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
            TEST_ASSERT_NOT_EQUAL_MESSAGE(next, s->rx_tail, "ring full during stress write");
            s->rx_buffer[s->rx_head] = write_val++;
            s->rx_head = next;
        }

        while (s->rx_tail != s->rx_head)
        {
            TEST_ASSERT_EQUAL_MESSAGE(read_val, s->rx_buffer[s->rx_tail], "ring corrupt on drain");
            read_val++;
            s->rx_tail = (s->rx_tail + 1) % RX_BUF_SIZE;
        }
    }

    TEST_ASSERT_EQUAL(s->rx_head, s->rx_tail); // empty after all cycles
}

// All four connection slots timeout simultaneously - every slot must be
// freed and none must corrupt a neighbour.
void stress_all_slots_timeout_simultaneously()
{
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i].state = ConnState::CONN_ACTIVE;
        conn_pool[i].pcb = nullptr;
        conn_pool[i].last_activity_ms = 0;
    }

    set_millis(CONN_TIMEOUT_MS);
    DeterministicAsyncTCP::check_timeouts();

    for (int i = 0; i < MAX_CONNS; i++)
    {
        TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[i].state);
        TEST_ASSERT_NULL(conn_pool[i].pcb);
        TEST_ASSERT_EQUAL(i, conn_pool[i].id); // id must not be smashed
    }
}

// Arms all slots, times them out, re-arms, times out again - 5 cycles.
// Verifies the pool recovers cleanly and check_timeouts is idempotent.
void stress_timeout_arm_recover_cycle()
{
    for (int cycle = 0; cycle < 5; cycle++)
    {
        for (int i = 0; i < MAX_CONNS; i++)
        {
            conn_pool[i].state = ConnState::CONN_ACTIVE;
            conn_pool[i].pcb = nullptr;
            conn_pool[i].last_activity_ms = 0;
        }

        set_millis((uint32_t)(CONN_TIMEOUT_MS * (cycle + 1)));
        DeterministicAsyncTCP::check_timeouts();

        for (int i = 0; i < MAX_CONNS; i++)
            TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[i].state);
    }
}

// Runs check_timeouts() 2000 times against a mix of free, active-fresh,
// and active-stale slots - verifies no crash and final state is correct.
void stress_check_timeouts_high_call_rate()
{
    conn_pool[0].state = ConnState::CONN_FREE;
    conn_pool[1].state = ConnState::CONN_ACTIVE;
    conn_pool[1].pcb = nullptr;
    conn_pool[1].last_activity_ms = 0;
    conn_pool[2].state = ConnState::CONN_ACTIVE;
    conn_pool[2].pcb = nullptr;
    conn_pool[2].last_activity_ms = CONN_TIMEOUT_MS; // diff = (now - TIMEOUT_MS) = 0 < TIMEOUT_MS
    conn_pool[3].state = ConnState::CONN_FREE;

    set_millis(CONN_TIMEOUT_MS); // slot 1 will expire, slot 2 won't

    for (int i = 0; i < 2000; i++)
        DeterministicAsyncTCP::check_timeouts();

    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[0].state);
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[1].state);   // expired
    TEST_ASSERT_EQUAL(ConnState::CONN_ACTIVE, (ConnState)conn_pool[2].state); // still fresh
    TEST_ASSERT_EQUAL(ConnState::CONN_FREE, (ConnState)conn_pool[3].state);
}

// Fills ring buffer with ascending bytes one byte at a time, checking
// capacity after each write, then drains one byte at a time and verifies.
void stress_ring_buffer_byte_by_byte_fill_and_drain()
{
    TcpConn *s = &conn_pool[0];
    s->rx_head = 0;
    s->rx_tail = 0;

    int written = 0;
    while (true)
    {
        size_t next = (s->rx_head + 1) % RX_BUF_SIZE;
        if (next == s->rx_tail)
            break; // full
        s->rx_buffer[s->rx_head] = (uint8_t)(written & 0xFF);
        s->rx_head = next;
        written++;
    }
    TEST_ASSERT_EQUAL(RX_BUF_SIZE - 1, written);

    int read = 0;
    while (s->rx_tail != s->rx_head)
    {
        TEST_ASSERT_EQUAL((uint8_t)(read & 0xFF), s->rx_buffer[s->rx_tail]);
        s->rx_tail = (s->rx_tail + 1) % RX_BUF_SIZE;
        read++;
    }
    TEST_ASSERT_EQUAL(written, read);
}

// ====================================================================
// Accept-rate throttle (connection-flood defense)
// ====================================================================

// Within one window, the first MAX accepts pass and the next is rejected.
void test_accept_throttle_blocks_over_budget()
{
    listener_accept_throttle_reset();
    for (int i = 0; i < DETWS_ACCEPT_THROTTLE_MAX; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed(0));
    TEST_ASSERT_FALSE(listener_accept_allowed(0)); // budget exhausted
}

// Crossing into the next window refills the budget.
void test_accept_throttle_window_refills()
{
    listener_accept_throttle_reset();
    for (int i = 0; i < DETWS_ACCEPT_THROTTLE_MAX; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed(10));
    TEST_ASSERT_FALSE(listener_accept_allowed(10));
    // One full window later the counter resets.
    TEST_ASSERT_TRUE(listener_accept_allowed(10 + DETWS_ACCEPT_THROTTLE_WINDOW_MS));
}

// The unsigned window math survives a millis() rollover near 2^32.
void test_accept_throttle_handles_rollover()
{
    listener_accept_throttle_reset();
    uint32_t near_max = 0xFFFFFFFFu - 5;
    TEST_ASSERT_TRUE(listener_accept_allowed(near_max));
    // Wrap past zero: elapsed = (small - near_max) wraps to a large window jump.
    TEST_ASSERT_TRUE(listener_accept_allowed(near_max + DETWS_ACCEPT_THROTTLE_WINDOW_MS));
}

// ====================================================================
// Per-IP accept-rate throttle (per-source connection-flood defense)
// ====================================================================

// Within one window, a single source IP gets MAX accepts then is rejected.
void test_per_ip_throttle_blocks_over_budget()
{
    listener_per_ip_throttle_reset();
    DetIp ip = v4w(0xC0A80005u); // 192.168.0.5
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_MAX; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&ip, 0));
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&ip, 0)); // this address's budget exhausted
}

// One noisy address being throttled does not affect a different address.
void test_per_ip_throttle_isolates_addresses()
{
    listener_per_ip_throttle_reset();
    DetIp noisy = v4w(0x0A000001u), quiet = v4w(0x0A000002u);
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_MAX; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&noisy, 0));
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&noisy, 0)); // noisy is blocked
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&quiet, 0));  // a different IP is unaffected
}

// Crossing into the next window refills that address's budget.
void test_per_ip_throttle_window_refills()
{
    listener_per_ip_throttle_reset();
    DetIp ip = v4w(0x0A000003u);
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_MAX; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&ip, 50));
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&ip, 50));
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&ip, 50 + DETWS_PER_IP_THROTTLE_WINDOW_MS));
}

// When the bucket table is full of distinct addresses, a brand-new address still
// gets through (the least-recently-started bucket is evicted - bounded memory).
void test_per_ip_throttle_evicts_when_full()
{
    listener_per_ip_throttle_reset();
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_SLOTS; i++)
    {
        DetIp ip = v4w(0xAC100001u + (uint32_t)i);
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&ip, 100));
    }
    DetIp fresh = v4w(0xDEADBEEFu);
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&fresh, 100)); // evicts an old bucket
}

// An unspecified source address is untrackable and is always allowed - it defers to
// the global throttle rather than being mis-tracked.
void test_per_ip_throttle_zero_ip_always_allowed()
{
    listener_per_ip_throttle_reset();
    DetIp none;
    none.family = DetIpFamily::DET_IP_NONE;
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_MAX + 5; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&none, 0));
}

// Distinct IPv6 peers keep independent budgets (the key is the full 128-bit address,
// so a v6 attacker cannot collapse many addresses onto one bucket).
void test_per_ip_throttle_v6_distinct()
{
    listener_per_ip_throttle_reset();
    DetIp a;
    a.family = DetIpFamily::DET_IP_NONE;
    DetIp b;
    b.family = DetIpFamily::DET_IP_NONE;
    TEST_ASSERT_TRUE(det_ip_parse("2001:db8::1", &a));
    TEST_ASSERT_TRUE(det_ip_parse("2001:db8::2", &b));
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_MAX; i++)
        TEST_ASSERT_TRUE(listener_accept_allowed_ip(&a, 0));
    TEST_ASSERT_FALSE(listener_accept_allowed_ip(&a, 0)); // a exhausted
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&b, 0));  // b has its own budget
}

// The per-IP window math survives a millis() rollover near 2^32.
void test_per_ip_throttle_handles_rollover()
{
    listener_per_ip_throttle_reset();
    DetIp ip = v4w(0x0A000009u);
    uint32_t near_max = 0xFFFFFFFFu - 5;
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&ip, near_max));
    TEST_ASSERT_TRUE(listener_accept_allowed_ip(&ip, near_max + DETWS_PER_IP_THROTTLE_WINDOW_MS));
}

// ====================================================================
// Source-IP allowlist (accept-time firewall)
// ====================================================================

// An empty allowlist allows every address (enabling the feature without rules is
// a no-op and cannot lock the device out).
void test_ip_allowlist_empty_allows_all()
{
    listener_ip_allowlist_reset();
    DetIp a = v4w(0xC0A8010Au), b = v4w(0x08080808u); // 192.168.1.10, 8.8.8.8
    DetIp none;
    none.family = DetIpFamily::DET_IP_NONE;
    TEST_ASSERT_TRUE(listener_ip_allowed(&a));
    TEST_ASSERT_TRUE(listener_ip_allowed(&b));
    TEST_ASSERT_TRUE(listener_ip_allowed(&none));
}

// A /32 rule admits exactly one host and rejects all others.
void test_ip_allowlist_host_match()
{
    listener_ip_allowlist_reset();
    DetIp net = v4w(0xC0A8010Au); // 192.168.1.10
    TEST_ASSERT_TRUE(listener_ip_allow_add(&net, 32));
    DetIp host = v4w(0xC0A8010Au), near = v4w(0xC0A8010Bu), far = v4w(0x0A000001u);
    TEST_ASSERT_TRUE(listener_ip_allowed(&host));
    TEST_ASSERT_FALSE(listener_ip_allowed(&near));
    TEST_ASSERT_FALSE(listener_ip_allowed(&far));
}

// A /24 rule admits the whole subnet and rejects addresses outside it.
void test_ip_allowlist_cidr_match()
{
    listener_ip_allowlist_reset();
    DetIp net = v4w(0xC0A80100u); // 192.168.1.0
    TEST_ASSERT_TRUE(listener_ip_allow_add(&net, 24));
    DetIp lo = v4w(0xC0A80101u), hi = v4w(0xC0A801FEu), out = v4w(0xC0A80201u);
    TEST_ASSERT_TRUE(listener_ip_allowed(&lo));
    TEST_ASSERT_TRUE(listener_ip_allowed(&hi));
    TEST_ASSERT_FALSE(listener_ip_allowed(&out));
}

// Host bits below the prefix are masked at compare time, so a network argument with
// stray host bits still matches the whole subnet.
void test_ip_allowlist_masks_host_bits()
{
    listener_ip_allowlist_reset();
    DetIp net = v4w(0xC0A80137u); // 192.168.1.55 as a /24
    TEST_ASSERT_TRUE(listener_ip_allow_add(&net, 24));
    DetIp lo = v4w(0xC0A80101u), hi = v4w(0xC0A801C8u);
    TEST_ASSERT_TRUE(listener_ip_allowed(&lo));
    TEST_ASSERT_TRUE(listener_ip_allowed(&hi));
}

// Multiple rules are OR-ed: an address matching any rule is allowed.
void test_ip_allowlist_multiple_rules()
{
    listener_ip_allowlist_reset();
    DetIp r1 = v4w(0x0A000000u), r2 = v4w(0xC0A80000u); // 10.0.0.0/8, 192.168.0.0/16
    TEST_ASSERT_TRUE(listener_ip_allow_add(&r1, 8));
    TEST_ASSERT_TRUE(listener_ip_allow_add(&r2, 16));
    DetIp a = v4w(0x0A010203u), b = v4w(0xC0A80505u), out = v4w(0xAC100001u);
    TEST_ASSERT_TRUE(listener_ip_allowed(&a));
    TEST_ASSERT_TRUE(listener_ip_allowed(&b));
    TEST_ASSERT_FALSE(listener_ip_allowed(&out));
}

// A /0 rule matches every address of its family.
void test_ip_allowlist_zero_prefix_matches_all()
{
    listener_ip_allowlist_reset();
    DetIp z = v4w(0u);
    TEST_ASSERT_TRUE(listener_ip_allow_add(&z, 0));
    DetIp a = v4w(0x01020304u), b = v4w(0xFFFFFFFFu);
    TEST_ASSERT_TRUE(listener_ip_allowed(&a));
    TEST_ASSERT_TRUE(listener_ip_allowed(&b));
}

// An IPv6 CIDR rule admits its v6 subnet and never a v4 peer (families are isolated).
void test_ip_allowlist_v6_cidr()
{
    listener_ip_allowlist_reset();
    TEST_ASSERT_TRUE(listener_ip_allow_add_cidr("2001:db8::/32"));
    DetIp in;
    in.family = DetIpFamily::DET_IP_NONE;
    DetIp out;
    out.family = DetIpFamily::DET_IP_NONE;
    TEST_ASSERT_TRUE(det_ip_parse("2001:db8:0:0:1234::abcd", &in));
    TEST_ASSERT_TRUE(det_ip_parse("2001:db9::1", &out));
    TEST_ASSERT_TRUE(listener_ip_allowed(&in));
    TEST_ASSERT_FALSE(listener_ip_allowed(&out));
    DetIp v4peer = v4w(0xC0A80101u);
    TEST_ASSERT_FALSE(listener_ip_allowed(&v4peer)); // a v4 peer never matches a v6 rule
}

// A prefix length above the family width is rejected.
void test_ip_allowlist_rejects_bad_prefix()
{
    listener_ip_allowlist_reset();
    DetIp net = v4w(0xC0A80100u);
    TEST_ASSERT_FALSE(listener_ip_allow_add(&net, 33));
}

// The rule table is bounded: it fills to capacity then refuses more rules.
void test_ip_allowlist_table_full()
{
    listener_ip_allowlist_reset();
    for (int i = 0; i < DETWS_IP_ALLOWLIST_SLOTS; i++)
    {
        DetIp r = v4w(0x0A000000u + (uint32_t)i);
        TEST_ASSERT_TRUE(listener_ip_allow_add(&r, 32));
    }
    DetIp overflow = v4w(0x0A010000u);
    TEST_ASSERT_FALSE(listener_ip_allow_add(&overflow, 32));
}

int main()
{
    UNITY_BEGIN();

    // Unit tests
    RUN_TEST(test_pool_capacity_is_four);
    RUN_TEST(test_rx_buffer_size_is_one_kb);
    RUN_TEST(test_timeout_constant_is_5000ms);
    RUN_TEST(test_all_slots_free_after_init);
    RUN_TEST(test_all_pcbs_null_after_init);
    RUN_TEST(test_all_ring_buffers_empty_after_init);
    RUN_TEST(test_slot_ids_match_indices);
    RUN_TEST(test_ring_empty_when_head_equals_tail);
    RUN_TEST(test_ring_wrap_at_boundary);
    RUN_TEST(test_ring_full_sentinel_one_slot_reserved);
    RUN_TEST(test_ring_can_store_size_minus_one_bytes);
    RUN_TEST(test_event_types_are_distinct);
    RUN_TEST(test_timeout_does_not_fire_on_free_slot);
    RUN_TEST(test_timeout_does_not_fire_before_deadline);
    RUN_TEST(test_timeout_fires_at_deadline);
    RUN_TEST(test_timeout_fires_only_on_stale_slots);
    RUN_TEST(test_init_succeeds_on_native);
    RUN_TEST(test_all_last_activity_ms_zero_after_init);
    RUN_TEST(test_queue_not_null_after_init);

    // Stress tests
    RUN_TEST(stress_ring_buffer_fill_drain_integrity);
    RUN_TEST(stress_ring_buffer_multi_cycle_no_corruption);
    RUN_TEST(stress_all_slots_timeout_simultaneously);
    RUN_TEST(stress_timeout_arm_recover_cycle);
    RUN_TEST(stress_check_timeouts_high_call_rate);
    RUN_TEST(stress_ring_buffer_byte_by_byte_fill_and_drain);

    // Accept-rate throttle
    RUN_TEST(test_accept_throttle_blocks_over_budget);
    RUN_TEST(test_accept_throttle_window_refills);
    RUN_TEST(test_accept_throttle_handles_rollover);

    // Per-IP accept-rate throttle
    RUN_TEST(test_per_ip_throttle_blocks_over_budget);
    RUN_TEST(test_per_ip_throttle_isolates_addresses);
    RUN_TEST(test_per_ip_throttle_window_refills);
    RUN_TEST(test_per_ip_throttle_evicts_when_full);
    RUN_TEST(test_per_ip_throttle_zero_ip_always_allowed);
    RUN_TEST(test_per_ip_throttle_v6_distinct);
    RUN_TEST(test_per_ip_throttle_handles_rollover);

    // Source-IP allowlist
    RUN_TEST(test_ip_allowlist_empty_allows_all);
    RUN_TEST(test_ip_allowlist_host_match);
    RUN_TEST(test_ip_allowlist_cidr_match);
    RUN_TEST(test_ip_allowlist_masks_host_bits);
    RUN_TEST(test_ip_allowlist_multiple_rules);
    RUN_TEST(test_ip_allowlist_zero_prefix_matches_all);
    RUN_TEST(test_ip_allowlist_v6_cidr);
    RUN_TEST(test_ip_allowlist_rejects_bad_prefix);
    RUN_TEST(test_ip_allowlist_table_full);

    return UNITY_END();
}

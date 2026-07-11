// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file listener.cpp
 * @brief Layer 4 (Listener) - TCP accept callback and port lifecycle.
 *
 * `listener_accept_cb` is the single lwIP accept callback registered for
 * every listener.  The listener index is embedded in the PCB user-data via
 * `tcp_arg(listen_pcb, (void*)(uintptr_t)idx)` so this one function handles
 * all ports without a lookup table.
 *
 * The non-static per-connection callbacks (lowlevel_recv_cb, lowlevel_sent_cb,
 * lowlevel_err_cb) are defined in tcp.cpp and declared extern here.
 * The transport layer's enqueue() helper calls listener_enqueue(), which is
 * defined in this file - that indirection breaks the circular header dependency
 * (listener.h includes tcp.h; tcp.cpp includes listener.h).
 */

#include "listener.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "lwip/tcp.h"
#include "network_drivers/tls/tls.h" // TLS handshake begin (self-stubbing)
#ifdef ARDUINO
#include "lwip/ip_addr.h"                   // ip_2_ip4 / ip4_addr_get_u32 for interface tagging
#include "lwip/priv/tcpip_priv.h"           // tcpip_api_call - marshal dynamic listener ops to tcpip_thread
#include "network_drivers/session/worker.h" // detws_worker_wake() - nudge the owning worker task
#endif
#include "services/clock.h" // detws_millis() pluggable monotonic clock (host-safe)
#include <Arduino.h>

// Listener pool - all storage in BSS.
Listener listener_pool[MAX_LISTENERS];

// Per-connection callbacks defined in tcp.cpp.
extern err_t lowlevel_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
extern err_t lowlevel_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);
extern void lowlevel_err_cb(void *arg, err_t err);

// ---------------------------------------------------------------------------
// Accept-rate throttle (fixed window, global). State persists across accepts.
// Always compiled (unit-testable); only consulted when the feature is enabled.
// ---------------------------------------------------------------------------

// Global accept-rate-limit state, owned by one instance (internal linkage): the fixed-window
// start and the accept count in the current window. One named owner, unreachable cross-TU.
struct AcceptThrottleCtx
{
    uint32_t window_start = 0;
    uint16_t count = 0;
};
static AcceptThrottleCtx s_accept;

bool listener_accept_allowed(uint32_t now_ms)
{
    // Unsigned subtraction wraps correctly across the millis() rollover.
    if ((uint32_t)(now_ms - s_accept.window_start) >= DETWS_ACCEPT_THROTTLE_WINDOW_MS)
    {
        s_accept.window_start = now_ms;
        s_accept.count = 0;
    }
    if (s_accept.count >= DETWS_ACCEPT_THROTTLE_MAX)
        return false;
    s_accept.count++;
    return true;
}

void listener_accept_throttle_reset(void)
{
    s_accept.window_start = 0;
    s_accept.count = 0;
}

// ---------------------------------------------------------------------------
// Per-IP accept-rate throttle (fixed window per source IPv4). A bounded BSS table
// of buckets - no heap. Always compiled (unit-testable); only consulted when the
// feature is enabled.
// ---------------------------------------------------------------------------

struct IpThrottleBucket
{
    DetIp addr;            ///< source address (family DetIpFamily::DET_IP_NONE marks an empty bucket).
    uint32_t window_start; ///< millis() at the start of this bucket's current window.
    uint16_t count;        ///< connections counted from this address in the window.
};
// Per-source-IP accept-throttle state, owned by one instance (internal linkage): the bounded
// bucket table keyed by source address. One named owner, unreachable from any other TU.
struct IpThrottleCtx
{
    IpThrottleBucket buckets[DETWS_PER_IP_THROTTLE_SLOTS];
};
static IpThrottleCtx s_iptt;

bool listener_accept_allowed_ip(const DetIp *ip, uint32_t now_ms)
{
    if (det_ip_is_unspecified(ip))
        return true; // untrackable source - defer to the global accept throttle

    int empty = -1, expired = -1, lru = 0;
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_SLOTS; i++)
    {
        IpThrottleBucket *b = &s_iptt.buckets[i];
        if (b->addr.family != DetIpFamily::DET_IP_NONE && det_ip_equal(&b->addr, ip))
        {
            // Unsigned subtraction wraps correctly across the millis() rollover.
            if ((uint32_t)(now_ms - b->window_start) >= DETWS_PER_IP_THROTTLE_WINDOW_MS)
            {
                b->window_start = now_ms;
                b->count = 0;
            }
            if (b->count >= DETWS_PER_IP_THROTTLE_MAX)
                return false;
            b->count++;
            return true;
        }
        if (b->addr.family == DetIpFamily::DET_IP_NONE)
        {
            if (empty < 0)
                empty = i;
        }
        else
        {
            if (expired < 0 && (uint32_t)(now_ms - b->window_start) >= DETWS_PER_IP_THROTTLE_WINDOW_MS)
                expired = i;
            // Track the oldest active bucket (largest elapsed) as the eviction victim.
            if ((uint32_t)(now_ms - b->window_start) > (uint32_t)(now_ms - s_iptt.buckets[lru].window_start))
                lru = i;
        }
    }

    // No bucket yet for this address: claim one - empty, else expired, else evict
    // the least-recently-started active bucket.
    int slot = (empty >= 0) ? empty : (expired >= 0) ? expired : lru;
    IpThrottleBucket *b = &s_iptt.buckets[slot];
    b->addr = *ip;
    b->window_start = now_ms;
    b->count = 1;
    return true; // first connection of a fresh window is always allowed
}

void listener_per_ip_throttle_reset(void)
{
    for (int i = 0; i < DETWS_PER_IP_THROTTLE_SLOTS; i++)
    {
        s_iptt.buckets[i].addr.family = DetIpFamily::DET_IP_NONE;
        s_iptt.buckets[i].window_start = 0;
        s_iptt.buckets[i].count = 0;
    }
}

// ---------------------------------------------------------------------------
// Source-IP allowlist (accept-time firewall). A bounded BSS table of CIDR rules
// in host byte order. Always compiled (unit-testable); only consulted when
// DETWS_ENABLE_IP_ALLOWLIST is set. An empty table allows everything so enabling
// the feature before adding rules cannot lock the device out.
// ---------------------------------------------------------------------------

struct IpAllowRule
{
    DetIp network; ///< network address (family DetIpFamily::DET_IP_V4 / V6; DetIpFamily::DET_IP_NONE marks unused).
    uint8_t prefix_len; ///< CIDR prefix length: 0..32 for v4, 0..128 for v6.
};
// IP allowlist state, owned by one instance (internal linkage): the CIDR rule table and its
// count (empty = allow all). One named owner, unreachable from any other translation unit.
struct IpAllowCtx
{
    IpAllowRule rules[DETWS_IP_ALLOWLIST_SLOTS];
    uint8_t count = 0;
};
static IpAllowCtx s_allow;

bool listener_ip_allow_add(const DetIp *network, uint8_t prefix_len)
{
    if (!network)
        return false;
    int bits =
        (network->family == DetIpFamily::DET_IP_V4) ? 32 : (network->family == DetIpFamily::DET_IP_V6 ? 128 : -1);
    if (bits < 0 || prefix_len > (uint8_t)bits)
        return false; // reject a malformed family or an over-long prefix
    if (s_allow.count >= DETWS_IP_ALLOWLIST_SLOTS)
        return false;
    s_allow.rules[s_allow.count].network = *network;
    s_allow.rules[s_allow.count].prefix_len = prefix_len;
    s_allow.count++;
    return true;
}

bool listener_ip_allow_add_cidr(const char *cidr)
{
    if (!cidr)
        return false;

    // Split "address/prefix" at the slash. The address half is copied into a bounded
    // buffer (a CIDR string is never longer than an address plus "/128") for the parser.
    char addr[DET_IP_STR_MAX];
    const char *slash = nullptr;
    size_t n = 0;
    for (const char *p = cidr; *p; p++)
    {
        if (*p == '/')
        {
            slash = p;
            break;
        }
        if (n + 1 >= sizeof(addr))
            return false; // address text too long to be valid
        addr[n++] = *p;
    }
    addr[n] = '\0';

    DetIp net;
    net.family = DetIpFamily::DET_IP_NONE;
    if (!det_ip_parse(addr, &net))
        return false;

    uint8_t width = (net.family == DetIpFamily::DET_IP_V4) ? 32 : 128;
    uint8_t prefix = width; // bare address -> host route
    if (slash)
    {
        // Parse the decimal prefix by hand (no stdlib in src/); reject empty or non-digit.
        uint32_t v = 0;
        const char *p = slash + 1;
        if (!*p)
            return false;
        for (; *p; p++)
        {
            if (*p < '0' || *p > '9')
                return false;
            v = v * 10 + (uint32_t)(*p - '0');
            if (v > width)
                return false; // out of range for the family
        }
        prefix = (uint8_t)v;
    }

    return listener_ip_allow_add(&net, prefix);
}

bool listener_ip_allowed(const DetIp *ip)
{
    if (s_allow.count == 0)
        return true; // no rules configured -> allow all (fail-open by design)
    for (uint8_t i = 0; i < s_allow.count; i++)
    {
        // det_ip_prefix_match requires the same family, so a v4 peer never matches a v6 rule.
        if (det_ip_prefix_match(ip, &s_allow.rules[i].network, s_allow.rules[i].prefix_len))
            return true;
    }
    return false;
}

void listener_ip_allowlist_reset(void)
{
    for (int i = 0; i < DETWS_IP_ALLOWLIST_SLOTS; i++)
        s_allow.rules[i].network.family = DetIpFamily::DET_IP_NONE;
    s_allow.count = 0;
}

#if DETWS_WORKER_COUNT > 1
// Per-worker event queues: each worker drains only its own queue, so connection
// slots partition across workers with no shared-queue contention. Static BSS, no
// heap. Created once (idempotent) before the first accept can fire.
// Per-worker event-queue state, owned by one instance (internal linkage): the static queue
// control blocks, their storage, and the queue handles. One named owner, unreachable cross-TU.
struct ListenerQueueCtx
{
    StaticQueue_t wq_struct[DETWS_WORKER_COUNT];
    uint8_t wq_storage[DETWS_WORKER_COUNT][EVT_QUEUE_DEPTH * sizeof(TcpEvt)];
    QueueHandle_t wq[DETWS_WORKER_COUNT] = {nullptr};
};
static ListenerQueueCtx s_lq;

void listener_worker_queues_init(void)
{
    for (int i = 0; i < DETWS_WORKER_COUNT; i++)
        if (!s_lq.wq[i])
            s_lq.wq[i] = xQueueCreateStatic(EVT_QUEUE_DEPTH, sizeof(TcpEvt), s_lq.wq_storage[i], &s_lq.wq_struct[i]);
}

QueueHandle_t listener_worker_queue(int worker_id)
{
    if (worker_id < 0 || worker_id >= DETWS_WORKER_COUNT)
        return nullptr;
    return s_lq.wq[worker_id];
}
#endif // DETWS_WORKER_COUNT > 1

bool listener_enqueue(uint8_t listener_id, const TcpEvt *evt)
{
#if DETWS_WORKER_COUNT > 1
    // Route by the slot's owner so the owning worker is the sole consumer.
    (void)listener_id;
    uint8_t owner = conn_pool[evt->slot_id].owner;
    if (owner >= DETWS_WORKER_COUNT || !s_lq.wq[owner])
        return false;
    if (xQueueSend(s_lq.wq[owner], evt, 0) != pdTRUE)
        return false;
#ifdef ARDUINO
    detws_worker_wake(owner); // nudge the owning worker so it services this now
#endif
#else
    if (listener_id >= MAX_LISTENERS)
        return false;
    Listener *lst = &listener_pool[listener_id];
    if (!lst->active || !lst->queue)
        return false;
    if (xQueueSend(lst->queue, evt, 0) != pdTRUE)
        return false;
#ifdef ARDUINO
    detws_worker_wake(0); // single worker owns every slot - nudge it now
#endif
#endif
    return true;
}

/**
 * @brief lwIP accept callback - single handler for all listener ports.
 *
 * @p arg carries the listener index cast to a pointer via
 * `tcp_arg(listen_pcb, (void*)(uintptr_t)idx)`.  Finds a free TcpConn slot,
 * sets its protocol, wires the per-connection callbacks, and posts EVT_CONNECT
 * to the owning listener's queue.  Rejects the connection with ERR_ABRT when
 * the pool is full - ERR_ABRT tells lwIP the PCB is already gone from our side.
 */
static err_t listener_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    if (err != ERR_OK || newpcb == nullptr)
        return ERR_VAL;

    uint8_t idx = (uint8_t)(uintptr_t)arg;
    if (idx >= MAX_LISTENERS)
        return ERR_VAL;
    Listener *lst = &listener_pool[idx];

#if DETWS_ENABLE_ACCEPT_THROTTLE
    // Connection-flood defense: drop accepts beyond the per-window budget before
    // claiming a pool slot or doing any per-connection work.
    if (!listener_accept_allowed(detws_millis()))
    {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }
#endif

#if DETWS_ENABLE_PER_IP_THROTTLE || DETWS_ENABLE_IP_ALLOWLIST
    // Resolve the peer's family-tagged source address once for the accept-time abuse
    // gates below - the full IPv4 or IPv6 address, never a lossy hash. On native
    // there is no real lwIP pcb, so it stays unspecified and the gates pass it
    // through; the host unit tests drive those gates directly with synthetic DetIp.
    DetIp remote;
    remote.family = DetIpFamily::DET_IP_NONE;
#ifdef ARDUINO
    det_lwip_to_detip(&newpcb->remote_ip, &remote);
#endif
#endif

#if DETWS_ENABLE_PER_IP_THROTTLE
    // Per-source-IP flood defense: drop accepts beyond one address's per-window
    // budget (the global throttle cannot tell one noisy client from many). Keyed on
    // the full address, so an IPv6 peer cannot spray a /64 past a per-address cap.
    if (!listener_accept_allowed_ip(&remote, detws_millis()))
    {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }
#endif

#if DETWS_ENABLE_IP_ALLOWLIST
    // Source-IP firewall: drop connections from addresses outside the configured
    // allowlist (an empty allowlist allows all, so this is a no-op until rules are
    // added). CIDR prefix match on the full v4/v6 address.
    if (!listener_ip_allowed(&remote))
    {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }
#endif

    int free_slot = -1;
    for (int i = 0; i < MAX_CONNS; i++)
    {
        if (conn_pool[i].state == CONN_FREE)
        {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1)
    {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }

    TcpConn *slot = &conn_pool[free_slot];
#if DETWS_WORKER_COUNT > 1
    // Round-robin the new connection across workers. Runs only in tcpip_thread,
    // so the counter needs no lock. Set BEFORE the state release store so a worker
    // that observes CONN_ACTIVE also sees the owner, and so the EVT_CONNECT below
    // routes to the owner's queue.
    static uint8_t s_next_owner = 0;
    slot->owner = s_next_owner;
    s_next_owner = (uint8_t)((s_next_owner + 1) % DETWS_WORKER_COUNT);
#else
    slot->owner = 0;
#endif
    slot->state = CONN_ACTIVE;
    slot->pcb = newpcb;
    slot->last_activity_ms = detws_millis();
    slot->rx_head = 0;
    slot->rx_tail = 0;
    slot->rx_acked = 0; // window-ack cursor starts level with an empty ring
    slot->listener_id = idx;
    slot->proto = lst->proto;

    // Tag the ingress interface for per-route STA/AP filtering. On ESP32 compare
    // the connection's local IP to the configured softAP IP; on native (no real
    // pcb IP) leave it unclassified for tests to set directly.
#ifdef ARDUINO
    {
        uint32_t lip = ip4_addr_get_u32(ip_2_ip4(&newpcb->local_ip));
        slot->iface = (detws_ap_ip != 0 && lip == detws_ap_ip) ? DetIface::DETIFACE_AP : DetIface::DETIFACE_STA;
    }
#else
    slot->iface = DetIface::DETIFACE_ANY;
#endif

    tcp_arg(newpcb, slot);

#if DETWS_ENABLE_TLS
    // TLS listeners begin a handshake immediately; the session loop pumps it.
    slot->tls = lst->tls ? 1 : 0;
    if (lst->tls)
        det_tls_conn_begin(free_slot);
#else
    slot->tls = 0;
#endif
    tcp_recv(newpcb, lowlevel_recv_cb);
    tcp_sent(newpcb, lowlevel_sent_cb);
    tcp_err(newpcb, lowlevel_err_cb);

    DETWS_OBS_TRANSITION((uint8_t)free_slot, CONN_FREE, CONN_ACTIVE, DET_CONN_R_ACCEPT);

    TcpEvt evt = {EVT_CONNECT, (uint8_t)free_slot, 0};
    if (!listener_enqueue(idx, &evt))
        DETWS_OBS_NOTICE((uint8_t)free_slot, CONN_ACTIVE, DET_CONN_R_DEFER_DROP);

    return ERR_OK;
}

#ifdef ARDUINO
static err_t listener_lwip_marshal(uint8_t idx, uint16_t port, bool create);
#endif

int32_t listener_add(uint8_t idx, uint16_t port, ConnProto proto, bool tls)
{
    if (idx >= MAX_LISTENERS)
        return -1;

    listener_stop(idx); // clean up if already active

#if DETWS_WORKER_COUNT > 1
    listener_worker_queues_init(); // create the per-worker event queues once (idempotent)
#endif

    Listener *lst = &listener_pool[idx];
    lst->port = port;
    lst->proto = proto;
    lst->tls = tls;

    lst->queue = xQueueCreateStatic(EVT_QUEUE_DEPTH, sizeof(TcpEvt), lst->_queue_storage, &lst->_queue_struct);
    if (!lst->queue)
        return -1;

#ifdef ARDUINO
    // Create the listening PCB in tcpip_thread. With lwIP core-locking (arduino-esp32
    // 3.x / IDF 5.x) a raw tcp_new/bind/listen from the app or worker task that calls
    // begin() asserts ("Required to lock TCPIP core functionality"), so marshal it -
    // the same path the dynamic listener uses. Fields the accept callback reads (proto,
    // queue) are set above, before the pcb can accept.
    if (listener_lwip_marshal(idx, port, true) != ERR_OK)
    {
        vQueueDelete(lst->queue);
        lst->queue = nullptr;
        return -1;
    }
#else
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb)
        return -1;

    err_t bind_err = tcp_bind(pcb, IP_ANY_TYPE, port);
    if (bind_err != ERR_OK)
    {
        tcp_abort(pcb);
        return -1;
    }

    lst->listen_pcb = tcp_listen_with_backlog(pcb, MAX_CONNS);
    if (!lst->listen_pcb)
    {
        tcp_abort(pcb);
        return -1;
    }

    tcp_arg(lst->listen_pcb, (void *)(uintptr_t)idx);
    tcp_accept(lst->listen_pcb, listener_accept_cb);
#endif
    lst->active = true;

    return 1;
}

void listener_stop(uint8_t idx)
{
    if (idx >= MAX_LISTENERS)
        return;
    Listener *lst = &listener_pool[idx];
    if (!lst->active)
        return;
    lst->active = false;
#ifdef ARDUINO
    listener_lwip_marshal(idx, 0, false); // close the listen pcb in tcpip_thread
#else
    lst->listen_pcb = nullptr; // host build: no real pcb to close (matches listener_stop_dynamic)
#endif
    if (lst->queue)
    {
        vQueueDelete(lst->queue);
        lst->queue = nullptr;
    }
}

void listener_stop_all()
{
    for (uint8_t i = 0; i < MAX_LISTENERS; i++)
        listener_stop(i);
}

// ---------------------------------------------------------------------------
// tcpip_thread-marshaled listener create / close. Raw lwIP tcp_new/bind/listen/close
// must run in tcpip_thread: with lwIP core-locking (arduino-esp32 3.x / IDF 5.x) a
// call from any other task asserts, and without it a call off tcpip_thread races the
// stack. Both listener_add/stop (from begin()) and the dynamic listeners (SSH `ssh -R`
// remote-forward, opened from a worker task) route through here via tcpip_api_call().
// ---------------------------------------------------------------------------

#ifdef ARDUINO
struct DetListenerCall
{
    struct tcpip_api_call_data base;
    uint8_t idx;
    uint16_t port;
    bool create; // true = new+bind+listen+accept, false = close the listen pcb
    err_t result;
};

// Runs in tcpip_thread. Creates or closes the listening PCB for listener_pool[idx].
static err_t listener_lwip_do(struct tcpip_api_call_data *c)
{
    DetListenerCall *k = (DetListenerCall *)c;
    Listener *lst = &listener_pool[k->idx];
    k->result = ERR_OK;
    if (k->create)
    {
        struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
        if (!pcb)
        {
            k->result = ERR_MEM;
            return ERR_OK;
        }
        if (tcp_bind(pcb, IP_ANY_TYPE, k->port) != ERR_OK)
        {
            tcp_abort(pcb);
            k->result = ERR_USE; // port already bound
            return ERR_OK;
        }
        struct tcp_pcb *lp = tcp_listen_with_backlog(pcb, MAX_CONNS);
        if (!lp)
        {
            tcp_abort(pcb); // tcp_listen did not consume pcb on failure
            k->result = ERR_MEM;
            return ERR_OK;
        }
        tcp_arg(lp, (void *)(uintptr_t)k->idx);
        tcp_accept(lp, listener_accept_cb);
        lst->listen_pcb = lp;
    }
    else if (lst->listen_pcb)
    {
        tcp_close(lst->listen_pcb);
        lst->listen_pcb = nullptr;
    }
    return ERR_OK;
}

static err_t listener_lwip_marshal(uint8_t idx, uint16_t port, bool create)
{
    DetListenerCall k = {};
    k.idx = idx;
    k.port = port;
    k.create = create;
    tcpip_api_call(listener_lwip_do, &k.base);
    return k.result;
}
#endif // ARDUINO

int32_t listener_add_dynamic(uint8_t idx, uint16_t port, ConnProto proto)
{
    if (idx >= MAX_LISTENERS)
        return -1;
    listener_stop_dynamic(idx); // clean up if this slot was already active

#if DETWS_WORKER_COUNT > 1
    listener_worker_queues_init(); // idempotent (xQueueCreateStatic is task-safe)
#endif

    Listener *lst = &listener_pool[idx];
    lst->port = port;
    lst->proto = proto;
    lst->tls = false; // forwarded ports are plaintext bridges

    lst->queue = xQueueCreateStatic(EVT_QUEUE_DEPTH, sizeof(TcpEvt), lst->_queue_storage, &lst->_queue_struct);
    if (!lst->queue)
        return -1;

#ifdef ARDUINO
    // Create the listening PCB in tcpip_thread. Fields the accept callback reads
    // (proto, queue) are set above, before the pcb can accept anything.
    if (listener_lwip_marshal(idx, port, true) != ERR_OK)
    {
        vQueueDelete(lst->queue);
        lst->queue = nullptr;
        return -1;
    }
#else
    lst->listen_pcb = nullptr; // native host: no lwIP, exercised via the accept-gate unit paths
#endif

    lst->active = true;
    return 1;
}

void listener_stop_dynamic(uint8_t idx)
{
    if (idx >= MAX_LISTENERS)
        return;
    Listener *lst = &listener_pool[idx];
    if (!lst->active)
        return;
    lst->active = false;
#ifdef ARDUINO
    listener_lwip_marshal(idx, 0, false); // close the listen pcb in tcpip_thread
#else
    lst->listen_pcb = nullptr;
#endif
    if (lst->queue)
    {
        vQueueDelete(lst->queue);
        lst->queue = nullptr;
    }
}

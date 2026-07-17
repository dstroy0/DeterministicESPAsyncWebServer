// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file edge_cache_proxy.cpp
 * @brief CDN edge-cache tier - server glue. See edge_cache_proxy.h.
 */

#include "services/edge_cache/edge_cache_proxy.h"

#if DETWS_ENABLE_EDGE_CACHE

#include "dwserver.h"                                             // DetWebServer, Middleware, MwResult, ChunkSource
#include "network_drivers/presentation/http_parser/http_parser.h" // HttpReq, http_get_header, http_pool
#include "network_drivers/presentation/presentation.h"            // detws_http_set_edge_poll
#include "network_drivers/transport/client.h"                     // det_client_*
#include "network_drivers/transport/tcp.h"                        // det_conn_active
#include "services/clock.h"                                       // detws_millis
#include "services/edge_cache/edge_fetch.h"
#if DETWS_ENABLE_DBM
#include "services/edge_cache/edge_cache_sd.h" // L2 SD tier
#endif
#include "services/http_client/http_client.h" // http_client_parse_url
#include "shared_primitives/mime.h"           // DET_MIME_TEXT_PLAIN
#include <stdio.h>
#include <string.h>

namespace
{
struct EdgeRouteMap
{
    bool used;
    char prefix[MAX_PATH_LEN];
    char origin_host[DETWS_EDGE_ORIGIN_URL_MAX];
    uint16_t origin_port;
};

struct EdgeFetchSlot
{
    bool used;
    EdgeFetch f;
    uint8_t client_slot;
    bool revalidate;
    EdgeEntry *reval_entry; // the stale entry being revalidated (nullptr for a plain miss)
    char canon[DETWS_EDGE_KEY_MAX];
};

struct EdgePending
{
    bool active;
    uint8_t fetch_idx;
};

// The ChunkSource ctx for a paged send; must outlive the response, so it lives in the owned Ctx.
struct EdgeServeCursor
{
    bool active;
    EdgeEntry *entry;
    uint32_t off, end;
};

// The single owned file-static: all of this subsystem's mutable state.
struct EdgeCacheProxyCtx
{
    DetWebServer *server;
    bool registered;
    EdgeCacheStore store;
    EdgeRouteMap maps[DETWS_EDGE_MAP_MAX];
    EdgeFetchSlot fetches[DETWS_EDGE_FETCH_SLOTS];
    EdgePending pending[MAX_CONNS];
    EdgeServeCursor serve[MAX_CONNS];
    EdgeFetchTransport transport;
    char reqbuf[MAX_PATH_LEN + MAX_QUERY_LEN + 256]; // scratch for one origin request line (freed by send)
#if DETWS_ENABLE_DBM
    DetwsDbm *l2;                      // the persistent L2 tier (nullptr = L1-only)
    uint8_t sd_buf[EDGE_SD_VALUE_MAX]; // serialize/deserialize scratch for one L2 value
#endif
};
EdgeCacheProxyCtx s_ctx;

#if DETWS_ENABLE_DBM
// L1 write-back hook: spill an evicted victim to L2 (edge_sd_put skips no-validator / oversize entries).
void edge_on_evict(void *ctx, const EdgeEntry *victim)
{
    (void)ctx;
    if (s_ctx.l2 && edge_sd_put(s_ctx.l2, victim, s_ctx.sd_buf, sizeof(s_ctx.sd_buf)))
        s_ctx.store.stats.l2_spills++;
}
#endif

// --- det_client transport seam -------------------------------------------------------------------
int t_open(void *c, const char *host, uint16_t port, uint32_t timeout)
{
    (void)c;
    return det_client_open(host, port, timeout);
}
bool t_send(void *c, int cid, const void *d, size_t l)
{
    (void)c;
    return det_client_send(cid, d, l);
}
size_t t_read(void *c, int cid, uint8_t *b, size_t cap)
{
    (void)c;
    return det_client_read(cid, b, cap);
}
bool t_closed(void *c, int cid)
{
    (void)c;
    return det_client_is_closed(cid);
}
void t_close(void *c, int cid)
{
    (void)c;
    det_client_close(cid);
}

// Request-header lookup used to (re)serialize the Vary secondary key; ctx is the client HttpReq.
const char *req_lookup(void *ctx, const char *name)
{
    return http_get_header((const HttpReq *)ctx, name);
}

EdgeRouteMap *map_match(const char *path)
{
    for (int i = 0; i < DETWS_EDGE_MAP_MAX; i++)
    {
        if (!s_ctx.maps[i].used)
            continue;
        size_t pl = strlen(s_ctx.maps[i].prefix);
        if (strncmp(path, s_ctx.maps[i].prefix, pl) == 0)
            return &s_ctx.maps[i];
    }
    return nullptr;
}

int alloc_fetch()
{
    for (int i = 0; i < DETWS_EDGE_FETCH_SLOTS; i++)
        if (!s_ctx.fetches[i].used)
            return i;
    return -1;
}

// The ChunkSource: page the cached body to the client. On completion (or exhaustion) release a
// transient passthrough entry (key ""). A dropped connection leaves the transient LRU-reclaimable.
size_t edge_chunk_source(uint8_t *buf, size_t cap, void *ctx)
{
    EdgeServeCursor *c = (EdgeServeCursor *)ctx;
    if (!c->active || !c->entry)
        return 0;
    size_t remaining = c->end - c->off;
    if (remaining == 0)
    {
        c->active = false;
        if (c->entry->key[0] == '\0') // transient passthrough entry -> free its slot
            edge_store_free_entry(&s_ctx.store, c->entry);
        c->entry = nullptr;
        return 0;
    }
    size_t n = remaining < cap ? remaining : cap;
    memcpy(buf, c->entry->body + c->off, n);
    c->off += n;
    return n;
}

// Serve a real cache entry (200), replaying its validators + Age, tagged with @p xcache.
void serve_hit(uint8_t slot, EdgeEntry *e, uint32_t now, const char *xcache)
{
    EdgeServeCursor *c = &s_ctx.serve[slot];
    c->active = true;
    c->entry = e;
    c->off = 0;
    c->end = e->body_len;
    s_ctx.server->add_response_header(slot, "X-Cache", xcache);
    if (e->etag[0])
        s_ctx.server->add_response_header(slot, "ETag", e->etag);
    if (e->last_modified[0])
        s_ctx.server->add_response_header(slot, "Last-Modified", e->last_modified);
    if (e->content_encoding[0])
        s_ctx.server->add_response_header(slot, "Content-Encoding", e->content_encoding);
    long age = edge_current_age(e->initial_age, e->insert_ms, now);
    if (age < 0)
        age = 0;
    char agebuf[12];
    snprintf(agebuf, sizeof(agebuf), "%ld", age);
    s_ctx.server->add_response_header(slot, "Age", agebuf);
    const char *ct = e->content_type[0] ? e->content_type : "application/octet-stream";
    s_ctx.server->send_chunked(slot, 200, ct, edge_chunk_source, c);
}

// Serve a non-cacheable / non-200 origin response through a transient unindexed store slot, so the
// serve source outlives the fetch (which the caller frees) and no-store content is never re-served.
void serve_passthrough(uint8_t slot, EdgeFetch *f)
{
    EdgeEntry *e = edge_store_alloc(&s_ctx.store, "", ""); // key "" -> never matched by a lookup
    if (!e)
    {
        s_ctx.server->send(slot, 502, DET_MIME_TEXT_PLAIN, "Bad Gateway");
        return;
    }
    s_ctx.store.stats.stores--; // a transient is not a cache store
    e->status = f->status;
    if (!edge_header_value((const char *)f->buf, f->head_len, "Content-Type", e->content_type, sizeof(e->content_type)))
        strncpy(e->content_type, "application/octet-stream", sizeof(e->content_type) - 1);
    edge_header_value((const char *)f->buf, f->head_len, "Content-Encoding", e->content_encoding,
                      sizeof(e->content_encoding));
    size_t bl = f->body_len;
    if (bl > DETWS_EDGE_BODY_MAX)
        bl = DETWS_EDGE_BODY_MAX;
    memcpy(e->body, f->buf + f->body_off, bl);
    e->body_len = (uint16_t)bl;

    EdgeServeCursor *c = &s_ctx.serve[slot];
    c->active = true;
    c->entry = e;
    c->off = 0;
    c->end = e->body_len;
    s_ctx.server->add_response_header(slot, "X-Cache", "MISS");
    if (e->content_encoding[0])
        s_ctx.server->add_response_header(slot, "Content-Encoding", e->content_encoding);
    const char *ct = e->content_type[0] ? e->content_type : "application/octet-stream";
    s_ctx.server->send_chunked(slot, e->status ? e->status : 200, ct, edge_chunk_source, c);
}

// Store a cacheable 200 response into a fresh entry and serve it.
void store_response(uint8_t slot, EdgeFetchSlot *fs, HttpReq *req, const DetwsCacheControl *cc, const char *vary_hdr,
                    uint32_t now)
{
    EdgeFetch *f = &fs->f;
    const char *head = (const char *)f->buf;
    size_t head_len = f->head_len;

    char vary_vals[DETWS_EDGE_VARY_MAX];
    edge_vary_serialize(vary_hdr[0] ? vary_hdr : nullptr, req_lookup, req, vary_vals, sizeof(vary_vals));

    EdgeEntry *e = edge_store_alloc(&s_ctx.store, fs->canon, vary_vals);
    if (!e)
    {
        serve_passthrough(slot, f);
        return;
    }
    e->status = 200;
    edge_header_value(head, head_len, "Content-Type", e->content_type, sizeof(e->content_type));
    edge_header_value(head, head_len, "Content-Encoding", e->content_encoding, sizeof(e->content_encoding));
    edge_header_value(head, head_len, "ETag", e->etag, sizeof(e->etag));
    edge_header_value(head, head_len, "Last-Modified", e->last_modified, sizeof(e->last_modified));
    if (vary_hdr[0] && strlen(vary_hdr) < sizeof(e->vary_names))
        memcpy(e->vary_names, vary_hdr, strlen(vary_hdr) + 1);

    size_t bl = f->body_len;
    if (bl > DETWS_EDGE_BODY_MAX)
        bl = DETWS_EDGE_BODY_MAX;
    memcpy(e->body, f->buf + f->body_off, bl);
    e->body_len = (uint16_t)bl;
    s_ctx.store.stats.bytes_stored += bl;

    int64_t date = -1;
    int64_t expires = -1;
    int64_t last_mod = -1;
    int32_t age = 0;
    char v[64];
    if (edge_header_value(head, head_len, "Date", v, sizeof(v)))
        date = edge_parse_http_date(v, strlen(v));
    if (edge_header_value(head, head_len, "Expires", v, sizeof(v)))
        expires = edge_parse_http_date(v, strlen(v));
    if (e->last_modified[0])
        last_mod = edge_parse_http_date(e->last_modified, strlen(e->last_modified));
    if (edge_header_value(head, head_len, "Age", v, sizeof(v)))
    {
        long a = 0;
        for (const char *p = v; *p >= '0' && *p <= '9'; p++)
            a = a * 10 + (*p - '0');
        age = (int32_t)a;
    }
    edge_entry_set_freshness(e, cc, /*shared=*/true, date, expires, last_mod, age, /*response_time=*/-1, now);
    serve_hit(slot, e, now, "MISS");
}

// A completed origin fetch: revalidation 304 / store 200 / pass through anything else.
void on_fetch_done(uint8_t slot, EdgeFetchSlot *fs, uint32_t now)
{
    EdgeFetch *f = &fs->f;
    const char *head = (const char *)f->buf;
    size_t head_len = f->head_len;
    HttpReq *req = &http_pool[slot];

    if (fs->revalidate && f->status == 304 && fs->reval_entry)
    {
        edge_apply_304(fs->reval_entry, head, head_len, -1, now);
        s_ctx.store.stats.revalidations_304++;
        serve_hit(slot, fs->reval_entry, now, "REVALIDATED");
        return;
    }
    if (f->status == 200)
    {
        DetwsCacheControl cc;
        cache_control_init(&cc);
        char v[128];
        if (edge_header_value(head, head_len, "Cache-Control", v, sizeof(v)))
            cache_control_parse(v, strlen(v), &cc);
        char vary_hdr[DETWS_EDGE_VARY_MAX];
        vary_hdr[0] = '\0';
        edge_header_value(head, head_len, "Vary", vary_hdr, sizeof(vary_hdr));
        if (edge_is_storeable(200, "GET", &cc, vary_hdr[0] ? vary_hdr : nullptr, f->body_len))
        {
            if (fs->revalidate && fs->reval_entry) // 200 on a revalidation replaces the stale entry
            {
                edge_store_free_entry(&s_ctx.store, fs->reval_entry);
                s_ctx.store.stats.replaces_200++;
            }
            store_response(slot, fs, req, &cc, vary_hdr, now);
            return;
        }
        serve_passthrough(slot, f); // 200 but not storeable
        return;
    }
    serve_passthrough(slot, f); // non-200 status
}

// Forward decls for the seam functions installed by det_edge_cache_enable().
MwResult edge_cache_mw(uint8_t slot, HttpReq *req);
bool edge_cache_poll(uint8_t slot);

bool start_fetch(uint8_t slot, HttpReq *req, EdgeRouteMap *m, const char *canon, EdgeEntry *reval, uint32_t now)
{
    int fi = alloc_fetch();
    if (fi < 0)
        return false;
    EdgeFetchSlot *fs = &s_ctx.fetches[fi];
    char cond[192];
    cond[0] = '\0';
    if (reval)
        edge_build_conditional(reval, cond, sizeof(cond));
    int rl =
        snprintf(s_ctx.reqbuf, sizeof(s_ctx.reqbuf),
                 "GET %s%s%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: DetWebServer-EdgeCache\r\nConnection: close\r\n%s\r\n",
                 req->path, req->query[0] ? "?" : "", req->query, m->origin_host, cond);
    if (rl <= 0 || (size_t)rl >= sizeof(s_ctx.reqbuf))
        return false;
    edge_fetch_begin(&fs->f, &s_ctx.transport, m->origin_host, m->origin_port, s_ctx.reqbuf, (size_t)rl, now);
    if (fs->f.st == EdgeFetchStatus::FAILED)
    {
        edge_fetch_end(&fs->f, &s_ctx.transport);
        return false;
    }
    fs->used = true;
    fs->client_slot = slot;
    fs->revalidate = (reval != nullptr);
    fs->reval_entry = reval;
    memcpy(fs->canon, canon, strlen(canon) + 1);
    s_ctx.pending[slot].active = true;
    s_ctx.pending[slot].fetch_idx = (uint8_t)fi;
    return true;
}

#if DETWS_ENABLE_DBM
// Promote a reboot-surviving entry from L2 into a fresh L1 slot, forced stale so the caller revalidates it
// (the monotonic insert time is meaningless across a reboot). @return the promoted entry, or nullptr.
EdgeEntry *try_promote_l2(const char *canon, uint32_t now)
{
    uint8_t digest[32];
    edge_key_digest(canon, strlen(canon), digest);
    EdgeEntry *e = edge_store_alloc(&s_ctx.store, canon, ""); // may evict + write-back an L1 victim first
    if (!e)
        return nullptr;
    if (!edge_sd_get(s_ctx.l2, digest, e, s_ctx.sd_buf, sizeof(s_ctx.sd_buf)) || strcmp(e->key, canon) != 0)
    {
        edge_store_free_entry(&s_ctx.store, e); // L2 miss or digest collision -> not promoted
        return nullptr;
    }
    e->lifetime_s = 0; // force stale: freshness is untrustworthy across a reboot -> caller revalidates
    e->initial_age = 0;
    e->date_epoch = e->expires_epoch = -1;
    e->age_hdr = 0;
    e->insert_ms = now;
    e->last_used_ms = now;
    s_ctx.store.stats.l2_promotes++;
    return e;
}
#endif

// The cache middleware: fresh hit -> serve; stale/miss -> start an async origin fetch (suspend); else
// fall through (fail open).
MwResult edge_cache_mw(uint8_t slot, HttpReq *req)
{
    if (!s_ctx.registered || !s_ctx.server || slot >= MAX_CONNS)
        return MwResult::MW_NEXT;
    bool is_get = strcmp(req->method, "GET") == 0;
    bool is_head = strcmp(req->method, "HEAD") == 0;
    if (!is_get && !is_head)
        return MwResult::MW_NEXT; // only cache safe methods
    if (http_get_header(req, "Authorization"))
        return MwResult::MW_NEXT; // never cache authorized/private requests
    EdgeRouteMap *m = map_match(req->path);
    if (!m)
        return MwResult::MW_NEXT; // not a mapped origin

    const char *host = http_get_header(req, "Host");
    if (!host)
        host = "";
    char canon[DETWS_EDGE_KEY_MAX];
    if (edge_key_canon("GET", host, req->path, req->query, /*include_query=*/true, canon, sizeof(canon)) == 0)
        return MwResult::MW_NEXT; // key too long -> uncacheable, fail open

    uint32_t now = detws_millis();
    EdgeEntry *e = edge_store_find(&s_ctx.store, canon, req_lookup, req, now);
    if (e && edge_entry_fresh(e, now))
    {
        s_ctx.store.stats.hits++;
        serve_hit(slot, e, now, "HIT");
        return MwResult::MW_HALT;
    }
#if DETWS_ENABLE_DBM
    if (!e && s_ctx.l2) // L1 miss: try promoting a reboot-surviving entry from L2 (force-stale -> revalidate)
        e = try_promote_l2(canon, now);
#endif
    s_ctx.store.stats.misses++;
    EdgeEntry *reval = (e && edge_entry_has_validator(e)) ? e : nullptr;
    if (!start_fetch(slot, req, m, canon, reval, now))
        return MwResult::MW_NEXT; // no fetch slot / origin open failed -> fail open to normal dispatch
    return MwResult::MW_HALT;     // client request suspended until the fetch completes
}

// Per-slot poll hook: drive an in-flight origin fetch, then serve. Returns true while it owns the slot.
bool edge_cache_poll(uint8_t slot)
{
    if (slot >= MAX_CONNS || !s_ctx.pending[slot].active)
        return false;
    uint8_t fi = s_ctx.pending[slot].fetch_idx;
    EdgeFetchSlot *fs = &s_ctx.fetches[fi];
    uint32_t now = detws_millis();

    if (!det_conn_active(slot)) // client vanished mid-fetch: abort
    {
        edge_fetch_end(&fs->f, &s_ctx.transport);
        fs->used = false;
        s_ctx.pending[slot].active = false;
        return true;
    }

    EdgeFetchStatus st = edge_fetch_pump(&fs->f, &s_ctx.transport, now);
    if (st == EdgeFetchStatus::PENDING)
        return true; // still receiving; owns the slot

    if (st == EdgeFetchStatus::DONE)
    {
        on_fetch_done(slot, fs, now);
    }
    else if (st == EdgeFetchStatus::FAILED && fs->revalidate && fs->reval_entry)
    {
        serve_hit(slot, fs->reval_entry, now, "STALE"); // stale-if-error: serve the last good copy
    }
    else // FAILED miss / OVERSIZE
    {
        s_ctx.server->send(slot, 502, DET_MIME_TEXT_PLAIN, "Bad Gateway");
    }
    edge_fetch_end(&fs->f, &s_ctx.transport);
    fs->used = false;
    s_ctx.pending[slot].active = false;
    return true;
}
} // namespace

// --- public API ----------------------------------------------------------------------------------

void det_edge_cache_enable(DetWebServer &server)
{
    s_ctx.server = &server;
    edge_store_init(&s_ctx.store);
    for (int i = 0; i < DETWS_EDGE_FETCH_SLOTS; i++)
    {
        s_ctx.fetches[i].used = false;
        s_ctx.fetches[i].f.cid = -1;
    }
    for (int i = 0; i < MAX_CONNS; i++)
    {
        s_ctx.pending[i].active = false;
        s_ctx.serve[i].active = false;
        s_ctx.serve[i].entry = nullptr;
    }
    s_ctx.transport.open = t_open;
    s_ctx.transport.send = t_send;
    s_ctx.transport.read = t_read;
    s_ctx.transport.closed = t_closed;
    s_ctx.transport.close = t_close;
    s_ctx.transport.ctx = nullptr;
#if DETWS_ENABLE_DBM
    s_ctx.store.on_evict = s_ctx.l2 ? edge_on_evict : nullptr; // re-arm write-back after edge_store_init
    s_ctx.store.evict_ctx = nullptr;
#endif
    if (!s_ctx.registered)
    {
        server.use(edge_cache_mw);
        detws_http_set_edge_poll(edge_cache_poll);
        s_ctx.registered = true;
    }
}

#if DETWS_ENABLE_DBM
void det_edge_cache_bind_sd(DetwsDbm *dbm)
{
    s_ctx.l2 = dbm;
    s_ctx.store.on_evict = dbm ? edge_on_evict : nullptr;
    s_ctx.store.evict_ctx = nullptr;
}
#endif

bool det_edge_cache_map(const char *path_prefix, const char *origin_base_url)
{
    if (!path_prefix || !origin_base_url)
        return false;
    if (strlen(path_prefix) >= sizeof(s_ctx.maps[0].prefix))
        return false;
    bool https = false;
    char host[DETWS_EDGE_ORIGIN_URL_MAX];
    uint16_t port = 80;
    char ignore_path[256];
    if (!http_client_parse_url(origin_base_url, &https, host, sizeof(host), &port, ignore_path, sizeof(ignore_path)))
        return false;
    if (https)
        return false; // v1: plaintext origins only (TLS origin is a follow-up)
    for (int i = 0; i < DETWS_EDGE_MAP_MAX; i++)
    {
        if (s_ctx.maps[i].used)
            continue;
        strncpy(s_ctx.maps[i].prefix, path_prefix, sizeof(s_ctx.maps[i].prefix) - 1);
        s_ctx.maps[i].prefix[sizeof(s_ctx.maps[i].prefix) - 1] = '\0';
        strncpy(s_ctx.maps[i].origin_host, host, sizeof(s_ctx.maps[i].origin_host) - 1);
        s_ctx.maps[i].origin_host[sizeof(s_ctx.maps[i].origin_host) - 1] = '\0';
        s_ctx.maps[i].origin_port = port;
        s_ctx.maps[i].used = true;
        return true;
    }
    return false; // map table full
}

void det_edge_cache_reset(void)
{
    edge_store_init(&s_ctx.store);
#if DETWS_ENABLE_DBM
    if (s_ctx.l2)
    {
        edge_sd_purge_all(s_ctx.l2);
        s_ctx.store.on_evict = edge_on_evict; // edge_store_init cleared it - re-arm the write-back hook
    }
#endif
    for (int i = 0; i < DETWS_EDGE_MAP_MAX; i++)
        s_ctx.maps[i].used = false;
}

bool det_edge_cache_purge(const char *canonical_key)
{
    if (!canonical_key)
        return false;
    bool purged = edge_store_purge(&s_ctx.store, canonical_key) > 0;
#if DETWS_ENABLE_DBM
    if (s_ctx.l2)
    {
        uint8_t digest[32];
        edge_key_digest(canonical_key, strlen(canonical_key), digest);
        if (edge_sd_del(s_ctx.l2, digest))
            purged = true;
    }
#endif
    return purged;
}

uint32_t det_edge_cache_purge_prefix(const char *path_prefix)
{
    if (!path_prefix)
        return 0;
    uint32_t n = edge_store_purge_prefix(&s_ctx.store, path_prefix);
#if DETWS_ENABLE_DBM
    if (s_ctx.l2)
        n += edge_sd_purge_prefix(s_ctx.l2, path_prefix, s_ctx.sd_buf, sizeof(s_ctx.sd_buf));
#endif
    return n;
}

void det_edge_cache_stats(EdgeCacheStats *out)
{
    if (out)
        *out = s_ctx.store.stats;
}

#endif // DETWS_ENABLE_EDGE_CACHE

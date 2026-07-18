// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for the WebDAV request handler's recursive filesystem operations
// (COPY / MOVE / DELETE of collections, RFC 4918). These exercise the real
// handler in dwserver.cpp against the directory-capable
// in-memory FS mock (test/mocks/FS.h, opt-in via mock_fs_tree_enable()), which
// the files-only mock could not cover - so recursion was previously only
// HW-verifiable.

#include "dwserver.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static DWS server;
static fs::FS davfs;

static void push_str(uint8_t slot, const char *s)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; s[i]; i++)
    {
        size_t next = (c->rx_head + 1) % RX_BUF_SIZE;
        c->rx_buffer[c->rx_head] = (uint8_t)s[i];
        c->rx_head = next;
    }
}

static void feed_and_handle(uint8_t slot, const char *req)
{
    push_str(slot, req);
    http_parse(slot);
    server.handle();
}

// Re-arm slot 0 for a fresh request on the (close-after-each-DAV-response) flow.
static void rearm()
{
    conn_pool[0] = {};
    conn_pool[0].id = 0;
    conn_pool[0].state = ConnState::CONN_ACTIVE;
    conn_pool[0].proto = ConnProto::PROTO_HTTP;
    conn_pool[0].pcb = &_mock_pcb;
    http_reset(0);
    tcp_capture_reset();
}

// --- tree helpers (assert directly on the in-memory FS) ---------------------
static void tree_put(const char *path, const char *content)
{
    fs::MockNode *n = fs::_tree_add(path, false);
    TEST_ASSERT_NOT_NULL(n);
    n->len = strlen(content);
    memcpy(n->data, content, n->len);
}
static void tree_mkdir(const char *path)
{
    TEST_ASSERT_NOT_NULL(fs::_tree_add(path, true));
}
static bool tree_has(const char *path)
{
    return fs::_tree_find(path) != nullptr;
}
static bool tree_is_dir(const char *path)
{
    fs::MockNode *n = fs::_tree_find(path);
    return n && n->is_dir;
}
static bool tree_content_eq(const char *path, const char *exp)
{
    fs::MockNode *n = fs::_tree_find(path);
    return n && !n->is_dir && n->len == strlen(exp) && memcmp(n->data, exp, n->len) == 0;
}
static bool dws_resp_status(int code)
{
    char want[20];
    snprintf(want, sizeof(want), "HTTP/1.1 %d", code);
    return strstr(tcp_captured(), want) != nullptr;
}

// Build a source collection /dav/src with a nested subcollection + files.
static void populate_src()
{
    tree_mkdir("/dav/src");
    tree_put("/dav/src/a.txt", "alpha");
    tree_put("/dav/src/b.txt", "bravo");
    tree_mkdir("/dav/src/sub");
    tree_put("/dav/src/sub/c.txt", "charlie");
}

void setUp()
{
    server = DWS();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = ConnState::CONN_ACTIVE;
        conn_pool[i].proto = ConnProto::PROTO_HTTP;
        conn_pool[i].pcb = &_mock_pcb;
        http_reset(i);
    }
    ws_init();
    dws_sse_init();
    tcp_capture_reset();
    fs::mock_fs_tree_enable(); // directory-capable, empty tree
    server.dav("/dav", davfs, "/dav");
}

void tearDown()
{
    tcp_capture_disable();
    fs::mock_fs_tree_disable();
}

// ====================================================================
// TESTS
// ====================================================================

// RFC 4918 9.8: COPY of a collection (Depth infinity, the default) copies the
// whole tree, byte-exact, including the nested subcollection.
void test_copy_collection_recursive()
{
    populate_src();
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/dst\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(201));
    TEST_ASSERT_TRUE(tree_is_dir("/dav/dst"));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/dst/a.txt", "alpha"));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/dst/b.txt", "bravo"));
    TEST_ASSERT_TRUE(tree_is_dir("/dav/dst/sub"));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/dst/sub/c.txt", "charlie"));
    // The source is left intact (COPY, not MOVE).
    TEST_ASSERT_TRUE(tree_content_eq("/dav/src/a.txt", "alpha"));
}

// RFC 4918 9.8.3: COPY with Depth: 0 copies only the collection, not its members.
void test_copy_collection_depth0_shallow()
{
    populate_src();
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/shallow\r\nDepth: 0\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(201));
    TEST_ASSERT_TRUE(tree_is_dir("/dav/shallow"));
    TEST_ASSERT_FALSE(tree_has("/dav/shallow/a.txt")); // members not copied
    TEST_ASSERT_FALSE(tree_has("/dav/shallow/sub"));
}

// RFC 4918 9.8.4: Overwrite defaults to T (replace -> 204); Overwrite: F over an
// existing destination is refused with 412.
void test_copy_overwrite_semantics()
{
    populate_src();
    tree_mkdir("/dav/dst"); // a pre-existing destination
    tree_put("/dav/dst/stale.txt", "old");

    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/dst\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(204));            // replaced
    TEST_ASSERT_FALSE(tree_has("/dav/dst/stale.txt")); // target cleared first
    TEST_ASSERT_TRUE(tree_content_eq("/dav/dst/a.txt", "alpha"));

    rearm();
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/dst\r\nOverwrite: F\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(412));
}

// MOVE of a collection re-paths the whole tree and removes the source.
void test_move_collection_recursive()
{
    populate_src();
    feed_and_handle(0, "MOVE /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/moved\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(201));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/moved/sub/c.txt", "charlie"));
    TEST_ASSERT_FALSE(tree_has("/dav/src"));
    TEST_ASSERT_FALSE(tree_has("/dav/src/sub/c.txt"));
}

// DELETE of a collection removes the whole tree (recursive), answering 204.
void test_delete_collection_recursive()
{
    populate_src();
    feed_and_handle(0, "DELETE /dav/src HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(204));
    TEST_ASSERT_FALSE(tree_has("/dav/src"));
    TEST_ASSERT_FALSE(tree_has("/dav/src/a.txt"));
    TEST_ASSERT_FALSE(tree_has("/dav/src/sub/c.txt"));
}

// PROPFIND Depth: 0 returns a 207 describing only the collection itself.
void test_propfind_depth0_collection_only()
{
    populate_src();
    feed_and_handle(0, "PROPFIND /dav/src HTTP/1.1\r\nHost: x\r\nDepth: 0\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(dws_resp_status(207));
    TEST_ASSERT_NOT_NULL(strstr(r, "/dav/src"));
    TEST_ASSERT_NULL(strstr(r, "a.txt")); // members are not listed at Depth 0
}

// PROPFIND Depth: 1 lists the collection and its immediate members.
void test_propfind_depth1_lists_members()
{
    populate_src();
    feed_and_handle(0, "PROPFIND /dav/src HTTP/1.1\r\nHost: x\r\nDepth: 1\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(dws_resp_status(207));
    TEST_ASSERT_NOT_NULL(strstr(r, "a.txt"));
    TEST_ASSERT_NOT_NULL(strstr(r, "b.txt"));
    TEST_ASSERT_NOT_NULL(strstr(r, "sub")); // the subcollection is a member
}

// MKCOL creates a collection (201); repeating it on an existing path is 405.
void test_mkcol_create_and_conflict()
{
    feed_and_handle(0, "MKCOL /dav/newdir HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(201));
    TEST_ASSERT_TRUE(tree_is_dir("/dav/newdir"));

    rearm();
    feed_and_handle(0, "MKCOL /dav/newdir HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(405)); // already exists
}

// DELETE of a single file removes just that file (204), leaving siblings intact.
void test_delete_single_file()
{
    populate_src();
    feed_and_handle(0, "DELETE /dav/src/a.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(204));
    TEST_ASSERT_FALSE(tree_has("/dav/src/a.txt"));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/src/b.txt", "bravo")); // sibling untouched
}

// OPTIONS advertises WebDAV: a DAV compliance-class header and an Allow list.
void test_options_advertises_dav()
{
    feed_and_handle(0, "OPTIONS /dav/ HTTP/1.1\r\nHost: x\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(dws_resp_status(200) || dws_resp_status(204));
    TEST_ASSERT_NOT_NULL(strstr(r, "DAV:"));     // compliance class header
    TEST_ASSERT_NOT_NULL(strstr(r, "PROPFIND")); // Allow lists the DAV methods
}

// GET through the DAV mount streams a stored file's bytes (file-serving path).
void test_get_file_through_mount()
{
    populate_src();
    feed_and_handle(0, "GET /dav/src/a.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(dws_resp_status(200));
    TEST_ASSERT_NOT_NULL(strstr(r, "alpha"));
}

// --- streaming-PUT helpers -------------------------------------------------
// Push `len` raw bytes into slot's rx ring (a body may exceed any single ring
// so feed it in chunks, draining after each - as the real transport does).
static void push_bytes(uint8_t slot, const uint8_t *b, size_t len)
{
    TcpConn *c = &conn_pool[slot];
    for (size_t i = 0; i < len; i++)
    {
        c->rx_buffer[c->rx_head] = b[i];
        c->rx_head = (c->rx_head + 1) % RX_BUF_SIZE;
    }
}

// Feed a complete PUT with an arbitrary-length body and run the handler. The
// body streams to the DAV file via dav_stream_put_*; chunked feeding keeps the
// ring from overflowing regardless of RX_BUF_SIZE.
static void feed_put(uint8_t slot, const char *path, const uint8_t *body, size_t n)
{
    char hdr[128];
    snprintf(hdr, sizeof(hdr), "PUT %s HTTP/1.1\r\nHost: x\r\nContent-Length: %u\r\n\r\n", path, (unsigned)n);
    push_str(slot, hdr);
    http_parse(slot);
    for (size_t off = 0; off < n;)
    {
        size_t chunk = n - off > 200 ? 200 : n - off;
        push_bytes(slot, body + off, chunk);
        http_parse(slot);
        off += chunk;
    }
    server.handle();
}

// A new file streams straight to disk and answers 201 Created, byte-exact.
void test_put_stream_create()
{
    const char *body = "hello world";
    feed_put(0, "/dav/up.txt", (const uint8_t *)body, strlen(body));
    TEST_ASSERT_TRUE(dws_resp_status(201));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/up.txt", "hello world"));
}

// PUT over an existing file truncates and answers 204 (existed).
void test_put_stream_overwrite()
{
    tree_put("/dav/up.txt", "stale contents");
    const char *body = "new";
    feed_put(0, "/dav/up.txt", (const uint8_t *)body, strlen(body));
    TEST_ASSERT_TRUE(dws_resp_status(204));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/up.txt", "new"));
}

// An empty PUT has no streamed body (Content-Length 0), so it takes the buffered
// fallback: create the file and answer 201.
void test_put_empty_buffered()
{
    feed_and_handle(0, "PUT /dav/empty.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 0\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(201));
    TEST_ASSERT_TRUE(tree_has("/dav/empty.txt"));
}

// A body larger than the mock node's buffer short-writes -> the sink flags an
// error and the handler answers 507 Insufficient Storage.
void test_put_stream_write_fails_507()
{
    static uint8_t big[2100];
    memset(big, 'A', sizeof(big)); // > MockNode::data (2048) -> write() short-returns
    feed_put(0, "/dav/big.txt", big, sizeof(big));
    TEST_ASSERT_TRUE(dws_resp_status(507));
}

// When the FS cannot open the target (here: the mock's node table is full), the
// sink never activates and the handler answers 409 Conflict.
void test_put_stream_open_fails_409()
{
    char p[24];
    for (int i = 0; i < 64; i++) // exhaust MockNode table (64 slots)
    {
        snprintf(p, sizeof(p), "/dav/f%d", i);
        TEST_ASSERT_NOT_NULL(fs::_tree_add(p, false));
    }
    const char *body = "abc";
    feed_put(0, "/dav/overflow.txt", (const uint8_t *)body, strlen(body));
    TEST_ASSERT_TRUE(dws_resp_status(409));
}

// A ".." in the target is rejected at the stream-begin resolve (so it never
// streams) and again by the handler: 403 Forbidden.
void test_put_stream_traversal_403()
{
    const char *body = "abc";
    feed_put(0, "/dav/../secret", (const uint8_t *)body, strlen(body));
    TEST_ASSERT_TRUE(dws_resp_status(403));
}

// The stream-begin hook fires for any bodied request: a non-PUT method and a
// path that matches no DAV route both decline the sink (and don't crash).
void test_put_stream_begin_declines()
{
    // Non-PUT with a body: begin sees method != PUT and declines.
    feed_and_handle(0, "POST /dav/x.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\n\r\nabc");
    rearm();
    // PUT to a path outside any DAV mount: begin finds no route and declines.
    const char *body = "abc";
    feed_put(0, "/nomatch/y.txt", (const uint8_t *)body, strlen(body));
    TEST_ASSERT_TRUE(dws_resp_status(404)); // no route -> not found
}

// A streamed PUT torn down before completion (peer reset) runs the abort hook,
// which closes the half-open file so the handle is not leaked.
void test_put_stream_abort()
{
    // Headers + a partial body: Content-Length promises 10, only 4 arrive.
    push_str(0, "PUT /dav/ab.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 10\r\n\r\nabcd");
    http_parse(0);
    TEST_ASSERT_TRUE(tree_has("/dav/ab.txt")); // begin opened (created) the file
    http_reset(0);                             // body_streaming && !COMPLETE -> abort hook
    // The file is still present (open created the node); the handle was closed.
    TEST_ASSERT_TRUE(tree_has("/dav/ab.txt"));
}

// LOCK issues an advisory token (200 + Lock-Token); UNLOCK answers 204.
void test_lock_unlock_advisory()
{
    populate_src();
    feed_and_handle(0, "LOCK /dav/src/a.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(dws_resp_status(200));
    TEST_ASSERT_NOT_NULL(strstr(r, "Lock-Token"));

    rearm();
    feed_and_handle(0, "UNLOCK /dav/src/a.txt HTTP/1.1\r\nHost: x\r\nLock-Token: <urn:x>\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(204));
}

// The WebDAV method error paths: bad/foreign/traversal destinations, missing sources, and a
// Depth: infinity PROPFIND rejection.
void test_webdav_error_paths()
{
    feed_and_handle(0, "DELETE /dav/nope HTTP/1.1\r\nHost: x\r\n\r\n"); // no such resource
    TEST_ASSERT_TRUE(dws_resp_status(404));

    rearm();
    populate_src();
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\n\r\n"); // no Destination
    TEST_ASSERT_TRUE(dws_resp_status(400));

    rearm();
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /other/x\r\n\r\n"); // foreign mount
    TEST_ASSERT_TRUE(dws_resp_status(502));

    rearm();
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/../x\r\n\r\n"); // traversal
    TEST_ASSERT_TRUE(dws_resp_status(403));

    rearm();
    feed_and_handle(0, "COPY /dav/gone HTTP/1.1\r\nHost: x\r\nDestination: /dav/x\r\n\r\n"); // no such source
    TEST_ASSERT_TRUE(dws_resp_status(404));

    rearm();
    tree_mkdir("/dav/mvdst");
    feed_and_handle(0, "MOVE /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/mvdst\r\n\r\n"); // replace existing
    TEST_ASSERT_TRUE(dws_resp_status(204));

    rearm();
    feed_and_handle(0, "PROPFIND /dav/nope HTTP/1.1\r\nHost: x\r\nDepth: 0\r\n\r\n"); // no such resource
    TEST_ASSERT_TRUE(dws_resp_status(404));

    rearm();
    populate_src();
    feed_and_handle(0, "PROPFIND /dav/src HTTP/1.1\r\nHost: x\r\nDepth: infinity\r\n\r\n"); // finite-depth only
    TEST_ASSERT_TRUE(dws_resp_status(403));
}

// A tree deeper than the recursion bound (8) is refused by both the recursive delete and copy,
// which fail closed (nothing removed) rather than overflow the stack.
void test_webdav_deep_tree_rejected()
{
    char p[300];
    int off = snprintf(p, sizeof p, "/dav/deep");
    tree_mkdir(p);
    for (int i = 0; i < 10; i++)
    {
        off += snprintf(p + off, sizeof(p) - off, "/l%d", i);
        tree_mkdir(p);
    }
    feed_and_handle(0, "DELETE /dav/deep HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(403));  // dav_rm_recursive refuses past depth 8
    TEST_ASSERT_TRUE(tree_has("/dav/deep")); // nothing was removed

    rearm();
    feed_and_handle(0, "COPY /dav/deep HTTP/1.1\r\nHost: x\r\nDestination: /dav/dcopy\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(409)); // dav_copy_recursive refuses past depth 8
}

// PROPFIND of a directory with more members than the listing cap (DWS_WEBDAV_MAX_ENTRIES) stops at
// the limit; PROPPATCH answers 207 for an existing resource and 404 for a missing one.
void test_webdav_propfind_limit_and_proppatch()
{
    tree_mkdir("/dav/big");
    for (int i = 0; i < 40; i++) // more than DWS_WEBDAV_MAX_ENTRIES (32)
    {
        char p[64];
        snprintf(p, sizeof p, "/dav/big/f%02d.txt", i);
        tree_put(p, "x");
    }
    feed_and_handle(0, "PROPFIND /dav/big HTTP/1.1\r\nHost: x\r\nDepth: 1\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(207)); // listing truncated at the entry cap

    rearm();
    tree_put("/dav/file.txt", "data");
    feed_and_handle(0, "PROPPATCH /dav/file.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 0\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(207)); // properties refused, request accepted

    rearm();
    feed_and_handle(0, "PROPPATCH /dav/nope HTTP/1.1\r\nHost: x\r\nContent-Length: 0\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(404));
}

// COPY when the FS node table is exhausted: the destination file / collection cannot be created, so
// the recursive copy fails closed (409) rather than emitting a partial tree.
void test_webdav_copy_fs_table_full()
{
    tree_put("/dav/f.txt", "data");
    tree_mkdir("/dav/d");
    char p[32];
    for (int i = 0; i < 100; i++) // fill every remaining node slot
    {
        snprintf(p, sizeof p, "/dav/p%03d", i);
        if (!fs::_tree_add(p, false))
            break;
    }
    feed_and_handle(0, "COPY /dav/f.txt HTTP/1.1\r\nHost: x\r\nDestination: /dav/fc\r\n\r\n"); // dst open("w") fails
    TEST_ASSERT_TRUE(dws_resp_status(409));

    rearm();
    feed_and_handle(0, "COPY /dav/d HTTP/1.1\r\nHost: x\r\nDestination: /dav/dc\r\n\r\n"); // mkdir(dst) fails
    TEST_ASSERT_TRUE(dws_resp_status(409));
}

// WebDAV method-handler edges: GET/HEAD on a missing resource (404) and on a collection
// (405, not a download); a COPY Destination with a trailing slash (the joined path's
// trailing '/' is stripped); and a buffered (empty-body) PUT whose file cannot be created
// because the FS node table is exhausted (409).
void test_webdav_get_put_dest_edges()
{
    feed_and_handle(0, "GET /dav/missing.txt HTTP/1.1\r\nHost: x\r\n\r\n"); // no such resource
    TEST_ASSERT_TRUE(dws_resp_status(404));

    rearm();
    feed_and_handle(0, "HEAD /dav/missing.txt HTTP/1.1\r\nHost: x\r\n\r\n"); // same open-fail guard
    TEST_ASSERT_TRUE(dws_resp_status(404));

    rearm();
    tree_mkdir("/dav/adir");
    feed_and_handle(0, "GET /dav/adir HTTP/1.1\r\nHost: x\r\n\r\n"); // GET on a collection
    TEST_ASSERT_TRUE(dws_resp_status(405));

    rearm();
    tree_put("/dav/f.txt", "hi");
    feed_and_handle(0, "COPY /dav/f.txt HTTP/1.1\r\nHost: x\r\nDestination: /dav/g.txt/\r\n\r\n"); // trailing slash
    TEST_ASSERT_TRUE(dws_resp_status(201));   // created at the slash-stripped path
    TEST_ASSERT_TRUE(tree_has("/dav/g.txt")); // no trailing-slash node

    rearm();
    char p[32];
    for (int i = 0; i < 100; i++) // exhaust the node table
    {
        snprintf(p, sizeof p, "/dav/q%03d", i);
        if (!fs::_tree_add(p, false))
            break;
    }
    feed_and_handle(0, "PUT /dav/newfile.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 0\r\n\r\n"); // open("w") fails
    TEST_ASSERT_TRUE(dws_resp_status(409));
}

// COPY whose destination filesystem path would overflow the 256-byte path buffer: a deep
// fs root plus a destination name exceeds it, so the destination dav_join fails -> 414. The
// destination join runs before the source-exists check, and the short source path under the
// same deep root still resolves, so the request reaches the COPY case (not a top-level 404).
void test_webdav_copy_dest_path_too_long_414()
{
    // 240-char fs root: a short source ("/s") still joins under 256, but root + any
    // real destination sub-path overflows the 256-byte join buffer.
    static char longroot[241];
    memset(longroot, 'r', sizeof(longroot) - 1);
    longroot[0] = '/';
    longroot[sizeof(longroot) - 1] = '\0';
    server.dav("/d2", davfs, longroot);

    char req[128];
    // dest sub-path "/destination_file_name.txt" (26 chars) + the 240-char root overflows 256.
    snprintf(req, sizeof(req), "COPY /d2/s HTTP/1.1\r\nHost: x\r\nDestination: /d2/destination_file_name.txt\r\n\r\n");
    feed_and_handle(0, req);
    TEST_ASSERT_TRUE(dws_resp_status(414));
}

// The recursive delete/copy helpers re-open each node defensively (a core can invalidate
// an open handle across the writes a copy makes). When a node that exists() reports cannot
// actually be opened, the helper fails closed rather than proceeding on a bad handle:
// DELETE of such a resource answers 403, and a COPY whose child cannot be opened answers 409.
void test_webdav_recursive_open_failure()
{
    // DELETE: the resource exists but its open() fails -> dav_rm_recursive bails -> 403.
    tree_put("/dav/locked.txt", "data");
    fs::_mock_open_fail_path() = "/dav/locked.txt";
    feed_and_handle(0, "DELETE /dav/locked.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(403));
    fs::_mock_open_fail_path() = "";
    TEST_ASSERT_TRUE(tree_has("/dav/locked.txt")); // nothing removed

    // COPY of a collection whose child cannot be opened during recursion -> 409.
    rearm();
    populate_src();
    fs::_mock_open_fail_path() = "/dav/src/a.txt"; // a child that openNextFile finds but open() rejects
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/cdst\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(409));
    fs::_mock_open_fail_path() = "";
}

// A request under a mount whose fs root is so long that root + the request sub-path would
// overflow the 256-byte resolve buffer: dav_resolve_path fails at the top -> 414 for the
// method (here GET), before any method-specific handling.
void test_webdav_source_path_too_long_414()
{
    static char longroot[255];
    memset(longroot, 'r', sizeof(longroot) - 1);
    longroot[0] = '/';
    longroot[sizeof(longroot) - 1] = '\0'; // 254-char fs root: root + "/x" == 256, the join fails
    server.dav("/d3", davfs, longroot);
    feed_and_handle(0, "GET /d3/x HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(414));
}

// dav() route registration edges: a prefix already ending in '*' is stored verbatim (no
// second wildcard appended), and once the route table is full a further dav() mount is
// dropped (fails closed) so a request to it is not served.
void test_webdav_dav_wildcard_and_route_full()
{
    // (a) A wildcard-terminated prefix is stored as-is; a request under it still routes.
    server.dav("/w*", davfs, "/w");
    tree_put("/w/f.txt", "hi");
    feed_and_handle(0, "GET /w/f.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(200));

    // (b) Fill the route table, then a further dav() mount is dropped -> its path 404s.
    rearm();
    for (int i = 0; i < MAX_ROUTES; i++)
    {
        char p[16];
        snprintf(p, sizeof p, "/r%d", i);
        server.dav(p, davfs, "/r");
    }
    server.dav("/dropped", davfs, "/d"); // table full -> dropped
    feed_and_handle(0, "GET /dropped/x HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(dws_resp_status(404)); // never registered
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_webdav_get_put_dest_edges);
    RUN_TEST(test_webdav_copy_dest_path_too_long_414);
    RUN_TEST(test_webdav_recursive_open_failure);
    RUN_TEST(test_webdav_source_path_too_long_414);
    RUN_TEST(test_webdav_dav_wildcard_and_route_full);
    RUN_TEST(test_webdav_error_paths);
    RUN_TEST(test_webdav_deep_tree_rejected);
    RUN_TEST(test_webdav_propfind_limit_and_proppatch);
    RUN_TEST(test_webdav_copy_fs_table_full);
    RUN_TEST(test_copy_collection_recursive);
    RUN_TEST(test_copy_collection_depth0_shallow);
    RUN_TEST(test_copy_overwrite_semantics);
    RUN_TEST(test_move_collection_recursive);
    RUN_TEST(test_delete_collection_recursive);
    RUN_TEST(test_propfind_depth0_collection_only);
    RUN_TEST(test_propfind_depth1_lists_members);
    RUN_TEST(test_mkcol_create_and_conflict);
    RUN_TEST(test_delete_single_file);
    RUN_TEST(test_options_advertises_dav);
    RUN_TEST(test_get_file_through_mount);
    RUN_TEST(test_put_stream_create);
    RUN_TEST(test_put_stream_overwrite);
    RUN_TEST(test_put_empty_buffered);
    RUN_TEST(test_put_stream_write_fails_507);
    RUN_TEST(test_put_stream_open_fails_409);
    RUN_TEST(test_put_stream_traversal_403);
    RUN_TEST(test_put_stream_begin_declines);
    RUN_TEST(test_put_stream_abort);
    RUN_TEST(test_lock_unlock_advisory);
    return UNITY_END();
}

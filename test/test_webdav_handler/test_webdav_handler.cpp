// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for the WebDAV request handler's recursive filesystem operations
// (COPY / MOVE / DELETE of collections, RFC 4918). These exercise the real
// handler in DeterministicESPAsyncWebServer.cpp against the directory-capable
// in-memory FS mock (test/mocks/FS.h, opt-in via mock_fs_tree_enable()), which
// the files-only mock could not cover - so recursion was previously only
// HW-verifiable.

#include "DeterministicESPAsyncWebServer.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

static DetWebServer server;
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
    conn_pool[0].state = CONN_ACTIVE;
    conn_pool[0].proto = PROTO_HTTP;
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
static bool resp_status(int code)
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
    server = DetWebServer();
    for (int i = 0; i < MAX_CONNS; i++)
    {
        conn_pool[i] = {};
        conn_pool[i].id = (uint8_t)i;
        conn_pool[i].state = CONN_ACTIVE;
        conn_pool[i].proto = PROTO_HTTP;
        conn_pool[i].pcb = &_mock_pcb;
        http_reset(i);
    }
    ws_init();
    sse_init();
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
    TEST_ASSERT_TRUE(resp_status(201));
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
    TEST_ASSERT_TRUE(resp_status(201));
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
    TEST_ASSERT_TRUE(resp_status(204));                // replaced
    TEST_ASSERT_FALSE(tree_has("/dav/dst/stale.txt")); // target cleared first
    TEST_ASSERT_TRUE(tree_content_eq("/dav/dst/a.txt", "alpha"));

    rearm();
    feed_and_handle(0, "COPY /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/dst\r\nOverwrite: F\r\n\r\n");
    TEST_ASSERT_TRUE(resp_status(412));
}

// MOVE of a collection re-paths the whole tree and removes the source.
void test_move_collection_recursive()
{
    populate_src();
    feed_and_handle(0, "MOVE /dav/src HTTP/1.1\r\nHost: x\r\nDestination: /dav/moved\r\n\r\n");
    TEST_ASSERT_TRUE(resp_status(201));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/moved/sub/c.txt", "charlie"));
    TEST_ASSERT_FALSE(tree_has("/dav/src"));
    TEST_ASSERT_FALSE(tree_has("/dav/src/sub/c.txt"));
}

// DELETE of a collection removes the whole tree (recursive), answering 204.
void test_delete_collection_recursive()
{
    populate_src();
    feed_and_handle(0, "DELETE /dav/src HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(resp_status(204));
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
    TEST_ASSERT_TRUE(resp_status(207));
    TEST_ASSERT_NOT_NULL(strstr(r, "/dav/src"));
    TEST_ASSERT_NULL(strstr(r, "a.txt")); // members are not listed at Depth 0
}

// PROPFIND Depth: 1 lists the collection and its immediate members.
void test_propfind_depth1_lists_members()
{
    populate_src();
    feed_and_handle(0, "PROPFIND /dav/src HTTP/1.1\r\nHost: x\r\nDepth: 1\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(resp_status(207));
    TEST_ASSERT_NOT_NULL(strstr(r, "a.txt"));
    TEST_ASSERT_NOT_NULL(strstr(r, "b.txt"));
    TEST_ASSERT_NOT_NULL(strstr(r, "sub")); // the subcollection is a member
}

// MKCOL creates a collection (201); repeating it on an existing path is 405.
void test_mkcol_create_and_conflict()
{
    feed_and_handle(0, "MKCOL /dav/newdir HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(resp_status(201));
    TEST_ASSERT_TRUE(tree_is_dir("/dav/newdir"));

    rearm();
    feed_and_handle(0, "MKCOL /dav/newdir HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(resp_status(405)); // already exists
}

// DELETE of a single file removes just that file (204), leaving siblings intact.
void test_delete_single_file()
{
    populate_src();
    feed_and_handle(0, "DELETE /dav/src/a.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    TEST_ASSERT_TRUE(resp_status(204));
    TEST_ASSERT_FALSE(tree_has("/dav/src/a.txt"));
    TEST_ASSERT_TRUE(tree_content_eq("/dav/src/b.txt", "bravo")); // sibling untouched
}

// OPTIONS advertises WebDAV: a DAV compliance-class header and an Allow list.
void test_options_advertises_dav()
{
    feed_and_handle(0, "OPTIONS /dav/ HTTP/1.1\r\nHost: x\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(resp_status(200) || resp_status(204));
    TEST_ASSERT_NOT_NULL(strstr(r, "DAV:"));     // compliance class header
    TEST_ASSERT_NOT_NULL(strstr(r, "PROPFIND")); // Allow lists the DAV methods
}

// GET through the DAV mount streams a stored file's bytes (file-serving path).
void test_get_file_through_mount()
{
    populate_src();
    feed_and_handle(0, "GET /dav/src/a.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(resp_status(200));
    TEST_ASSERT_NOT_NULL(strstr(r, "alpha"));
}

// LOCK issues an advisory token (200 + Lock-Token); UNLOCK answers 204.
void test_lock_unlock_advisory()
{
    populate_src();
    feed_and_handle(0, "LOCK /dav/src/a.txt HTTP/1.1\r\nHost: x\r\n\r\n");
    const char *r = tcp_captured();
    TEST_ASSERT_TRUE(resp_status(200));
    TEST_ASSERT_NOT_NULL(strstr(r, "Lock-Token"));

    rearm();
    feed_and_handle(0, "UNLOCK /dav/src/a.txt HTTP/1.1\r\nHost: x\r\nLock-Token: <urn:x>\r\n\r\n");
    TEST_ASSERT_TRUE(resp_status(204));
}

int main()
{
    UNITY_BEGIN();
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
    RUN_TEST(test_lock_unlock_advisory);
    return UNITY_END();
}

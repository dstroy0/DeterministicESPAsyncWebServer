// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the WebDAV server core (services/webdav): method classification,
// header parsing, XML escaping, and the 207 Multi-Status builder. No FS/sockets.

#include "services/webdav/webdav.h"
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static bool contains(const char *hay, const char *needle)
{
    return strstr(hay, needle) != nullptr;
}

// ---------------------------------------------------------------------------
// Method classification
// ---------------------------------------------------------------------------

void test_method_classification()
{
    TEST_ASSERT_EQUAL_INT(DAV_M_OPTIONS, webdav_method("OPTIONS"));
    TEST_ASSERT_EQUAL_INT(DAV_M_PROPFIND, webdav_method("PROPFIND"));
    TEST_ASSERT_EQUAL_INT(DAV_M_PROPPATCH, webdav_method("PROPPATCH"));
    TEST_ASSERT_EQUAL_INT(DAV_M_MKCOL, webdav_method("MKCOL"));
    TEST_ASSERT_EQUAL_INT(DAV_M_COPY, webdav_method("COPY"));
    TEST_ASSERT_EQUAL_INT(DAV_M_MOVE, webdav_method("MOVE"));
    TEST_ASSERT_EQUAL_INT(DAV_M_LOCK, webdav_method("LOCK"));
    TEST_ASSERT_EQUAL_INT(DAV_M_UNLOCK, webdav_method("UNLOCK"));
    TEST_ASSERT_EQUAL_INT(DAV_M_PUT, webdav_method("PUT"));
    TEST_ASSERT_EQUAL_INT(DAV_M_GET, webdav_method("GET"));
    TEST_ASSERT_EQUAL_INT(DAV_M_DELETE, webdav_method("DELETE"));
    TEST_ASSERT_EQUAL_INT(DAV_M_UNSUPPORTED, webdav_method("BREW"));
    TEST_ASSERT_EQUAL_INT(DAV_M_UNSUPPORTED, webdav_method(nullptr));
}

// Fail-closed guards on the pure builders: xml_escape zero-cap + plain-char truncation,
// dest_path null arguments, and the multi-status / proppatch builders returning the input
// length unchanged (ms_entry) or 0 (proppatch) when the output buffer can't hold the element.
void test_webdav_builder_guards()
{
    char out[256];
    TEST_ASSERT_EQUAL_size_t(0, webdav_xml_escape(out, 0, "x")); // zero cap
    // A plain (unescaped) run that overruns a tiny buffer stops mid-copy, still NUL-terminated.
    char tiny[4];
    webdav_xml_escape(tiny, sizeof(tiny), "abcdefgh");
    TEST_ASSERT_TRUE(strlen(tiny) < sizeof(tiny));

    TEST_ASSERT_FALSE(webdav_dest_path(nullptr, out, sizeof(out)));  // null destination
    TEST_ASSERT_FALSE(webdav_dest_path("/x", nullptr, sizeof(out))); // null out
    TEST_ASSERT_FALSE(webdav_dest_path("/x", out, 0));               // zero cap

    // ms_entry: an href whose escaped form + fixed markup overruns the internal 512-byte
    // element scratch returns the input length unchanged (atomic: nothing committed).
    char href[256];
    memset(href, 'a', 255);
    href[255] = '\0';
    char buf[4096];
    TEST_ASSERT_EQUAL_size_t(7, webdav_ms_entry(buf, sizeof(buf), 7, href, false, 42, nullptr, "text/plain"));

    // proppatch: a cap that holds the preamble but not the closing envelope -> 0.
    char small[200];
    TEST_ASSERT_EQUAL_size_t(0, webdav_proppatch_ms(small, sizeof(small), "/x", "", 0));
}

// ---------------------------------------------------------------------------
// Depth header
// ---------------------------------------------------------------------------

void test_depth_parsing()
{
    TEST_ASSERT_EQUAL_INT(0, webdav_depth("0", 1));
    TEST_ASSERT_EQUAL_INT(1, webdav_depth("1", 0));
    TEST_ASSERT_EQUAL_INT(DAV_DEPTH_INFINITY, webdav_depth("infinity", 0));
    TEST_ASSERT_EQUAL_INT(1, webdav_depth(nullptr, 1)); // absent -> default
    TEST_ASSERT_EQUAL_INT(7, webdav_depth("bogus", 7)); // unrecognized -> default
}

// ---------------------------------------------------------------------------
// XML escaping
// ---------------------------------------------------------------------------

void test_xml_escape()
{
    char out[128];
    size_t n = webdav_xml_escape(out, sizeof(out), "a&b<c>d\"e'f");
    TEST_ASSERT_EQUAL_STRING("a&amp;b&lt;c&gt;d&quot;e&apos;f", out);
    TEST_ASSERT_EQUAL_size_t(strlen(out), n);
}

void test_xml_escape_truncates_safely()
{
    char out[8];
    webdav_xml_escape(out, sizeof(out), "<<<<<<<<"); // each '<' is 4 chars escaped
    TEST_ASSERT_TRUE(strlen(out) < sizeof(out));     // never overruns
    TEST_ASSERT_EQUAL_STRING("&lt;", out);           // one full entity fits
}

// ---------------------------------------------------------------------------
// Destination header
// ---------------------------------------------------------------------------

void test_dest_absolute_uri()
{
    char out[128];
    TEST_ASSERT_TRUE(webdav_dest_path("http://host:8080/dav/new name.txt", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("/dav/new name.txt", out); // %20 not present here
}

void test_dest_percent_decoded()
{
    char out[128];
    TEST_ASSERT_TRUE(webdav_dest_path("http://h/dav/a%20b%2Fc.txt", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("/dav/a b/c.txt", out);
}

void test_dest_abs_path()
{
    char out[128];
    TEST_ASSERT_TRUE(webdav_dest_path("/dav/x.txt", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("/dav/x.txt", out);
}

void test_dest_rejects_malformed()
{
    char out[128];
    TEST_ASSERT_FALSE(webdav_dest_path("dav/x.txt", out, sizeof(out)));       // not absolute
    TEST_ASSERT_FALSE(webdav_dest_path("http://hostonly", out, sizeof(out))); // no path
    TEST_ASSERT_FALSE(webdav_dest_path("/bad%zz", out, sizeof(out)));         // bad escape
}

// ---------------------------------------------------------------------------
// 207 Multi-Status builder
// ---------------------------------------------------------------------------

void test_multistatus_file_and_collection()
{
    char buf[1024];
    size_t len = 0;
    len = webdav_ms_begin(buf, sizeof(buf), len);
    len = webdav_ms_entry(buf, sizeof(buf), len, "/dav/", true, 0, "Mon, 01 Jan 2026 00:00:00 GMT", "");
    len = webdav_ms_entry(buf, sizeof(buf), len, "/dav/readme.txt", false, 42, "Mon, 01 Jan 2026 00:00:00 GMT",
                          "text/plain");
    len = webdav_ms_end(buf, sizeof(buf), len);
    TEST_ASSERT_EQUAL_size_t(strlen(buf), len);

    TEST_ASSERT_TRUE(contains(buf, "<D:multistatus xmlns:D=\"DAV:\">"));
    TEST_ASSERT_TRUE(contains(buf, "<D:href>/dav/</D:href>"));
    TEST_ASSERT_TRUE(contains(buf, "<D:collection/>")); // the directory entry
    TEST_ASSERT_TRUE(contains(buf, "<D:href>/dav/readme.txt</D:href>"));
    TEST_ASSERT_TRUE(contains(buf, "<D:getcontentlength>42</D:getcontentlength>"));
    TEST_ASSERT_TRUE(contains(buf, "<D:getcontenttype>text/plain</D:getcontenttype>"));
    TEST_ASSERT_TRUE(contains(buf, "</D:multistatus>"));

    // A file entry has no <D:collection/>; the collection has no content length.
    const char *file_resp = strstr(buf, "/dav/readme.txt");
    TEST_ASSERT_FALSE(contains(file_resp, "<D:collection/>"));
}

void test_multistatus_escapes_href()
{
    char buf[512];
    size_t len = 0;
    len = webdav_ms_begin(buf, sizeof(buf), len);
    len = webdav_ms_entry(buf, sizeof(buf), len, "/dav/a&b.txt", false, 1, "", "text/plain");
    len = webdav_ms_end(buf, sizeof(buf), len);
    TEST_ASSERT_TRUE(contains(buf, "<D:href>/dav/a&amp;b.txt</D:href>"));
}

void test_multistatus_entry_stops_when_full()
{
    char buf[80]; // only room for the prolog + open tag, not a whole <response>
    size_t len = 0;
    len = webdav_ms_begin(buf, sizeof(buf), len);
    size_t before = len;
    size_t after = webdav_ms_entry(buf, sizeof(buf), before, "/dav/x.txt", false, 1, "", "text/plain");
    TEST_ASSERT_EQUAL_size_t(before, after); // did not fit -> unchanged, no partial element
    TEST_ASSERT_TRUE(strlen(buf) <= sizeof(buf));
}

// ---------------------------------------------------------------------------
// PROPPATCH 207 builder (read-only properties -> 403)
// ---------------------------------------------------------------------------

// Common scaffold checks: well-formed 207 wrapper around a 403 propstat.
static void assert_proppatch_envelope(const char *buf, size_t len)
{
    TEST_ASSERT_EQUAL_size_t(strlen(buf), len);
    TEST_ASSERT_TRUE(contains(buf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>"));
    TEST_ASSERT_TRUE(contains(buf, "<D:multistatus xmlns:D=\"DAV:\">"));
    TEST_ASSERT_TRUE(contains(buf, "<D:status>HTTP/1.1 403 Forbidden</D:status>"));
    TEST_ASSERT_TRUE(contains(buf, "</D:multistatus>"));
}

void test_proppatch_windows_timestamp()
{
    // The PROPPATCH macOS Finder / Windows Explorer send after a PUT.
    const char *body = "<?xml version=\"1.0\"?>\n"
                       "<D:propertyupdate xmlns:D=\"DAV:\" xmlns:Z=\"urn:schemas-microsoft-com:\">"
                       "<D:set><D:prop>"
                       "<Z:Win32LastModifiedTime>Tue, 06 Jan 2026 00:00:00 GMT</Z:Win32LastModifiedTime>"
                       "</D:prop></D:set></D:propertyupdate>";
    char buf[1024];
    size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/dav/file.txt", body, strlen(body));
    assert_proppatch_envelope(buf, len);
    TEST_ASSERT_TRUE(contains(buf, "<D:href>/dav/file.txt</D:href>"));
    // property echoed self-closed with its prefix + xmlns intact
    TEST_ASSERT_TRUE(contains(buf, "<Z:Win32LastModifiedTime>") == false); // not as an open tag with content
    TEST_ASSERT_TRUE(contains(buf, "<Z:Win32LastModifiedTime/>"));
    // wrappers must NOT be echoed as properties
    TEST_ASSERT_FALSE(contains(buf, "<D:set/>"));
    TEST_ASSERT_FALSE(contains(buf, "<D:propertyupdate/>"));
}

void test_proppatch_multiple_and_self_closed()
{
    const char *body = "<D:propertyupdate xmlns:D=\"DAV:\"><D:set><D:prop>"
                       "<D:getlastmodified/>"
                       "<author xmlns=\"http://ns.example/\">x</author>"
                       "</D:prop></D:set></D:propertyupdate>";
    char buf[1024];
    size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/dav/d/", body, strlen(body));
    assert_proppatch_envelope(buf, len);
    TEST_ASSERT_TRUE(contains(buf, "<D:getlastmodified/>"));
    TEST_ASSERT_TRUE(contains(buf, "<author xmlns=\"http://ns.example/\"/>")); // default-ns prop, value dropped
}

void test_proppatch_remove_block()
{
    const char *body = "<D:propertyupdate xmlns:D=\"DAV:\" xmlns:Z=\"urn:x\">"
                       "<D:remove><D:prop><Z:custom/></D:prop></D:remove></D:propertyupdate>";
    char buf[512];
    size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/dav/f", body, strlen(body));
    assert_proppatch_envelope(buf, len);
    TEST_ASSERT_TRUE(contains(buf, "<Z:custom/>")); // properties in <remove> are refused too
}

void test_proppatch_escapes_href()
{
    const char *body = "<D:propertyupdate xmlns:D=\"DAV:\"><D:set><D:prop><D:displayname/></D:prop></D:set>"
                       "</D:propertyupdate>";
    char buf[512];
    size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/dav/a&b", body, strlen(body));
    assert_proppatch_envelope(buf, len);
    TEST_ASSERT_TRUE(contains(buf, "<D:href>/dav/a&amp;b</D:href>"));
}

void test_proppatch_empty_body_is_valid()
{
    char buf[512];
    size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/dav/f", "", 0);
    assert_proppatch_envelope(buf, len); // still a well-formed 207 with an empty prop
    TEST_ASSERT_TRUE(contains(buf, "<D:prop>"));
}

void test_proppatch_rejects_injection()
{
    // A property tag carrying a stray '<' must not be echoed (no XML injection).
    const char *body = "<D:propertyupdate xmlns:D=\"DAV:\"><D:set><D:prop>"
                       "<evil attr=\"<broken\"/>"
                       "</D:prop></D:set></D:propertyupdate>";
    char buf[512];
    size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/dav/f", body, strlen(body));
    assert_proppatch_envelope(buf, len);
    TEST_ASSERT_FALSE(contains(buf, "<broken")); // the malformed tag was skipped
}

void test_proppatch_fuzz_bounded()
{
    // Throw random and partial-XML bytes at the scanner: it must always stay in
    // bounds, never emit a partial document, and leave a NUL-terminated buffer.
    uint32_t rng = 0xC0FFEEu;
    char body[128];
    char buf[1024];
    for (int iter = 0; iter < 20000; iter++)
    {
        rng ^= rng << 13;
        rng ^= rng >> 17;
        rng ^= rng << 5;
        size_t n = rng % sizeof(body);
        for (size_t i = 0; i < n; i++)
        {
            rng ^= rng << 13;
            rng ^= rng >> 17;
            rng ^= rng << 5;
            // bias toward XML punctuation so the scanner's tag paths are exercised
            const char *alpha = "<>/:= \"abcZD?!prop";
            body[i] = (rng & 3) ? alpha[(rng >> 2) % 18] : (char)(rng & 0xFF);
        }
        size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/x", body, n);
        TEST_ASSERT_TRUE(len <= sizeof(buf));
        if (len)
        {
            TEST_ASSERT_EQUAL_size_t(len, strlen(buf));
            TEST_ASSERT_TRUE(contains(buf, "</D:multistatus>")); // complete document or len 0
        }
        else
            TEST_ASSERT_EQUAL_size_t(0, strlen(buf));
    }
}

void test_proppatch_stops_when_full()
{
    const char *body = "<D:propertyupdate xmlns:D=\"DAV:\"><D:set><D:prop><D:displayname/></D:prop></D:set>"
                       "</D:propertyupdate>";
    char buf[40]; // too small for the whole document
    size_t len = webdav_proppatch_ms(buf, sizeof(buf), "/dav/file.txt", body, strlen(body));
    TEST_ASSERT_EQUAL_size_t(0, len);            // signals "did not fit"
    TEST_ASSERT_TRUE(strlen(buf) < sizeof(buf)); // no overrun
}

void test_method_all_including_head()
{
    TEST_ASSERT_EQUAL_INT(DAV_M_HEAD, webdav_method("HEAD"));
    TEST_ASSERT_EQUAL_INT(DAV_M_OPTIONS, webdav_method("OPTIONS"));
    TEST_ASSERT_EQUAL_INT(DAV_M_MKCOL, webdav_method("MKCOL"));
    TEST_ASSERT_EQUAL_INT(DAV_M_COPY, webdav_method("COPY"));
    TEST_ASSERT_EQUAL_INT(DAV_M_MOVE, webdav_method("MOVE"));
    TEST_ASSERT_EQUAL_INT(DAV_M_LOCK, webdav_method("LOCK"));
    TEST_ASSERT_EQUAL_INT(DAV_M_UNLOCK, webdav_method("UNLOCK"));
    TEST_ASSERT_EQUAL_INT(DAV_M_UNSUPPORTED, webdav_method("BOGUS"));
    TEST_ASSERT_EQUAL_INT(DAV_M_UNSUPPORTED, webdav_method(nullptr));
}

void test_depth_and_dest_path_guards()
{
    TEST_ASSERT_EQUAL_INT(7, webdav_depth(nullptr, 7)); // null -> default
    TEST_ASSERT_EQUAL_INT(7, webdav_depth("", 7));      // empty -> default
    TEST_ASSERT_EQUAL_INT(0, webdav_depth("0", 7));
    TEST_ASSERT_EQUAL_INT(1, webdav_depth("1", 7));
    char out[64];
    // Malformed %-escape in the destination path fails closed.
    TEST_ASSERT_FALSE(webdav_dest_path("http://host/a%zzb", out, sizeof(out)));
    // A path that overflows the output buffer fails closed.
    TEST_ASSERT_FALSE(webdav_dest_path("http://host/aaaaaaaaaaaaaaaaaaaaaaaaaaaa", out, 8));
}

void test_ms_entry_content_type_overflow()
{
    char buf[1024];
    size_t len = 0;
    char ct[420];
    for (int i = 0; i < 419; i++)
        ct[i] = 'x';
    ct[419] = '\0';
    // An oversized content-type overflows the internal element buffer -> len unchanged (atomic no-op).
    size_t r = webdav_ms_entry(buf, sizeof(buf), len, "/f.txt", false, 100, "Mon, 01 Jan 2026 00:00:00 GMT", ct);
    TEST_ASSERT_EQUAL_size_t(len, r);
}

void test_ms_entry_mtime_and_tiny_buf()
{
    char buf[1024];
    char mtime[460];
    for (int i = 0; i < 459; i++)
        mtime[i] = 'm';
    mtime[459] = '\0';
    // Oversized mtime overflows the element buffer -> len unchanged.
    TEST_ASSERT_EQUAL_size_t(0, webdav_ms_entry(buf, sizeof(buf), 0, "/f.txt", false, 100, mtime, "text/plain"));
    // A well-formed entry that does not fit the OUTPUT buffer leaves len unchanged (atomic commit fails).
    char tiny[40];
    TEST_ASSERT_EQUAL_size_t(0, webdav_ms_entry(tiny, sizeof(tiny), 0, "/f.txt", false, 100, "", "text/plain"));
}

void test_proppatch_ms_echo()
{
    char buf[512];
    // A self-closed property with trailing whitespace exercises the open-tag trim.
    const char *body = "<D:propertyupdate><D:set><D:prop><D:author  /></D:prop></D:set></D:propertyupdate>";
    size_t n = webdav_proppatch_ms(buf, sizeof(buf), "/file.txt", body, strlen(body));
    TEST_ASSERT_TRUE(n > 0);
    // A tiny buffer makes the leading append fail -> 0.
    TEST_ASSERT_EQUAL_size_t(0, webdav_proppatch_ms(buf, 20, "/file.txt", body, strlen(body)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_method_classification);
    RUN_TEST(test_webdav_builder_guards);
    RUN_TEST(test_depth_parsing);
    RUN_TEST(test_xml_escape);
    RUN_TEST(test_xml_escape_truncates_safely);
    RUN_TEST(test_dest_absolute_uri);
    RUN_TEST(test_dest_percent_decoded);
    RUN_TEST(test_dest_abs_path);
    RUN_TEST(test_dest_rejects_malformed);
    RUN_TEST(test_multistatus_file_and_collection);
    RUN_TEST(test_multistatus_escapes_href);
    RUN_TEST(test_multistatus_entry_stops_when_full);
    RUN_TEST(test_proppatch_windows_timestamp);
    RUN_TEST(test_proppatch_multiple_and_self_closed);
    RUN_TEST(test_proppatch_remove_block);
    RUN_TEST(test_proppatch_escapes_href);
    RUN_TEST(test_proppatch_empty_body_is_valid);
    RUN_TEST(test_proppatch_rejects_injection);
    RUN_TEST(test_proppatch_fuzz_bounded);
    RUN_TEST(test_proppatch_stops_when_full);
    RUN_TEST(test_method_all_including_head);
    RUN_TEST(test_depth_and_dest_path_guards);
    RUN_TEST(test_ms_entry_content_type_overflow);
    RUN_TEST(test_ms_entry_mtime_and_tiny_buf);
    RUN_TEST(test_proppatch_ms_echo);
    return UNITY_END();
}

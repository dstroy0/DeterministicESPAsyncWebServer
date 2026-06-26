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
    TEST_ASSERT_EQUAL_INT(DAV_M_UNSUPPORTED, webdav_method("BREW"));
    TEST_ASSERT_EQUAL_INT(DAV_M_UNSUPPORTED, webdav_method(nullptr));
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

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_method_classification);
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
    return UNITY_END();
}

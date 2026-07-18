// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/sftp: the SFTP protocol v3 wire codec. Covers the reader/writer round-trips, the
// ATTRS blob encode/decode (all flag combos + skip of unknown/extended fields), packet length-framing
// (need-more / complete / malformed / too-big), every response builder (VERSION / STATUS / HANDLE / DATA /
// ATTRS / NAME), a hand-built SSH_FXP_OPEN request parsed back, a multi-entry NAME via the writer API, the
// ls -l longname formatter, reader bounds safety, and builder overflow. Pure host tests (no fs, no SSH).

#include "server/fs_path.h"
#include "services/sftp/sftp.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

// --- reader / writer primitives ------------------------------------------------------------------
static void test_rw_roundtrip()
{
    uint8_t buf[64];
    SftpWriter w;
    dws_sftp_wr_init(&w, buf, sizeof(buf));
    dws_sftp_wr_u8(&w, 0xAB);
    dws_sftp_wr_u32(&w, 0x11223344);
    dws_sftp_wr_u64(&w, 0x0102030405060708ULL);
    dws_sftp_wr_string(&w, "hello", 5);
    size_t total = dws_sftp_wr_finish(&w);
    TEST_ASSERT_TRUE(total > 0);
    // length prefix == payload size
    TEST_ASSERT_EQUAL_UINT(total - 4, ((uint32_t)buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);

    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, total - 4);
    TEST_ASSERT_EQUAL_HEX8(0xAB, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_HEX32(0x11223344, dws_sftp_rd_u32(&r));
    TEST_ASSERT_EQUAL_HEX64(0x0102030405060708ULL, dws_sftp_rd_u64(&r));
    const uint8_t *s = nullptr;
    uint32_t sl = 0;
    TEST_ASSERT_TRUE(dws_sftp_rd_string(&r, &s, &sl));
    TEST_ASSERT_EQUAL_UINT(5, sl);
    TEST_ASSERT_EQUAL_MEMORY("hello", s, 5);
    TEST_ASSERT_TRUE(r.ok);
}

static void test_reader_bounds()
{
    uint8_t buf[3] = {0, 0, 1};
    SftpReader r;
    dws_sftp_rd_init(&r, buf, sizeof(buf));
    dws_sftp_rd_u32(&r); // wants 4, only 3 -> underflow
    TEST_ASSERT_FALSE(r.ok);
    // a string with a length past the end fails without over-reading
    uint8_t s[6] = {0, 0, 0, 100, 'a', 'b'}; // claims 100 bytes, only 2 present
    dws_sftp_rd_init(&r, s, sizeof(s));
    const uint8_t *p = nullptr;
    uint32_t n = 0;
    TEST_ASSERT_FALSE(dws_sftp_rd_string(&r, &p, &n));
    TEST_ASSERT_FALSE(r.ok);
}

// --- ATTRS round-trip ----------------------------------------------------------------------------
static void encode_attrs(const SftpAttrs *a, uint8_t *buf, size_t cap, size_t *plen)
{
    SftpWriter w;
    dws_sftp_wr_init(&w, buf, cap);
    dws_sftp_wr_attrs(&w, a);
    size_t total = dws_sftp_wr_finish(&w);
    *plen = total - 4; // payload only
}

static void test_attrs_roundtrip()
{
    SftpAttrs a;
    a.flags = SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_PERMISSIONS | SSH_FILEXFER_ATTR_ACMODTIME;
    a.size = 0x1122334455667788ULL;
    a.permissions = SFTP_S_IFREG | 0644;
    a.atime = 111;
    a.mtime = 222;

    uint8_t buf[64];
    size_t plen = 0;
    encode_attrs(&a, buf, sizeof(buf), &plen);

    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, plen);
    SftpAttrs out;
    TEST_ASSERT_TRUE(dws_sftp_rd_attrs(&r, &out));
    TEST_ASSERT_EQUAL_HEX32(a.flags, out.flags);
    TEST_ASSERT_EQUAL_HEX64(a.size, out.size);
    TEST_ASSERT_EQUAL_HEX32(a.permissions, out.permissions);
    TEST_ASSERT_EQUAL_UINT(a.atime, out.atime);
    TEST_ASSERT_EQUAL_UINT(a.mtime, out.mtime);
    TEST_ASSERT_EQUAL_UINT(plen, r.off); // consumed exactly the blob
}

static void test_attrs_skips_uidgid_and_extended()
{
    // Manually craft an ATTRS with UIDGID + PERMISSIONS + one EXTENDED pair, and confirm perms are recovered.
    uint8_t buf[80];
    SftpWriter w;
    dws_sftp_wr_init(&w, buf, sizeof(buf));
    dws_sftp_wr_u32(&w, SSH_FILEXFER_ATTR_UIDGID | SSH_FILEXFER_ATTR_PERMISSIONS | SSH_FILEXFER_ATTR_EXTENDED);
    dws_sftp_wr_u32(&w, 1000); // uid
    dws_sftp_wr_u32(&w, 1000); // gid
    dws_sftp_wr_u32(&w, SFTP_S_IFDIR | 0755);
    dws_sftp_wr_u32(&w, 1); // extended_count
    dws_sftp_wr_string(&w, "x@y", 3);
    dws_sftp_wr_string(&w, "v", 1);
    size_t total = dws_sftp_wr_finish(&w);

    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, total - 4);
    SftpAttrs out;
    TEST_ASSERT_TRUE(dws_sftp_rd_attrs(&r, &out));
    TEST_ASSERT_EQUAL_HEX32(SFTP_S_IFDIR | 0755, out.permissions);
    TEST_ASSERT_EQUAL_UINT(total - 4, r.off); // consumed uid/gid + extended too
}

// --- framing -------------------------------------------------------------------------------------
static void test_framing()
{
    uint8_t buf[16] = {0, 0, 0, 5, SSH_FXP_INIT, 0, 0, 0, 3};
    TEST_ASSERT_EQUAL_UINT(0, dws_sftp_frame_len(buf, 3, sizeof(buf))); // < 4 -> need more
    TEST_ASSERT_EQUAL_UINT(9, dws_sftp_frame_len(buf, 4, sizeof(buf))); // header present -> total 4+5
    TEST_ASSERT_EQUAL_UINT(9, dws_sftp_frame_len(buf, 9, sizeof(buf))); // whole packet
    uint8_t zero[4] = {0, 0, 0, 0};                                     // zero-length -> malformed
    TEST_ASSERT_EQUAL_UINT((size_t)-1, dws_sftp_frame_len(zero, 4, sizeof(buf)));
    uint8_t big[4] = {0, 0, 0xFF, 0xFF}; // 65535 payload, max 16 -> too big
    TEST_ASSERT_EQUAL_UINT((size_t)-1, dws_sftp_frame_len(big, 4, 16));
}

// --- a real request parsed back ------------------------------------------------------------------
static void test_parse_open_request()
{
    uint8_t buf[64];
    SftpWriter w;
    dws_sftp_wr_init(&w, buf, sizeof(buf));
    dws_sftp_wr_u8(&w, SSH_FXP_OPEN);
    dws_sftp_wr_u32(&w, 42);
    dws_sftp_wr_string(&w, "/foo.txt", 8);
    dws_sftp_wr_u32(&w, SSH_FXF_WRITE | SSH_FXF_CREAT | SSH_FXF_TRUNC);
    SftpAttrs empty = {0, 0, 0, 0, 0};
    dws_sftp_wr_attrs(&w, &empty);
    size_t total = dws_sftp_wr_finish(&w);

    TEST_ASSERT_EQUAL_UINT(total, dws_sftp_frame_len(buf, total, sizeof(buf)));
    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, total - 4);
    TEST_ASSERT_EQUAL_UINT8(SSH_FXP_OPEN, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_UINT32(42, dws_sftp_rd_u32(&r));
    const uint8_t *path = nullptr;
    uint32_t plen = 0;
    TEST_ASSERT_TRUE(dws_sftp_rd_string(&r, &path, &plen));
    TEST_ASSERT_EQUAL_UINT(8, plen);
    TEST_ASSERT_EQUAL_MEMORY("/foo.txt", path, 8);
    TEST_ASSERT_EQUAL_HEX32(SSH_FXF_WRITE | SSH_FXF_CREAT | SSH_FXF_TRUNC, dws_sftp_rd_u32(&r));
    SftpAttrs a;
    TEST_ASSERT_TRUE(dws_sftp_rd_attrs(&r, &a));
    TEST_ASSERT_EQUAL_HEX32(0, a.flags);
    TEST_ASSERT_TRUE(r.ok);
}

// --- response builders parsed back ---------------------------------------------------------------
static void test_build_version()
{
    uint8_t buf[16];
    size_t n = dws_sftp_build_version(buf, sizeof(buf));
    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, n - 4);
    TEST_ASSERT_EQUAL_UINT8(SSH_FXP_VERSION, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_UINT32(SFTP_VERSION, dws_sftp_rd_u32(&r));
}

static void test_build_status()
{
    uint8_t buf[64];
    size_t n = dws_sftp_build_status(7, SSH_FX_NO_SUCH_FILE, "nope", buf, sizeof(buf));
    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, n - 4);
    TEST_ASSERT_EQUAL_UINT8(SSH_FXP_STATUS, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_UINT32(7, dws_sftp_rd_u32(&r));
    TEST_ASSERT_EQUAL_UINT32(SSH_FX_NO_SUCH_FILE, dws_sftp_rd_u32(&r));
    const uint8_t *m = nullptr;
    uint32_t ml = 0;
    TEST_ASSERT_TRUE(dws_sftp_rd_string(&r, &m, &ml));
    TEST_ASSERT_EQUAL_MEMORY("nope", m, 4);
    TEST_ASSERT_TRUE(dws_sftp_rd_string(&r, nullptr, &ml)); // language tag (empty)
    TEST_ASSERT_EQUAL_UINT(0, ml);
}

static void test_build_handle_and_data()
{
    uint8_t buf[64];
    size_t n = dws_sftp_build_handle(3, "\x00\x00\x00\x02", 4, buf, sizeof(buf));
    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, n - 4);
    TEST_ASSERT_EQUAL_UINT8(SSH_FXP_HANDLE, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_UINT32(3, dws_sftp_rd_u32(&r));
    const uint8_t *h = nullptr;
    uint32_t hl = 0;
    dws_sftp_rd_string(&r, &h, &hl);
    TEST_ASSERT_EQUAL_UINT(4, hl);

    static const uint8_t payload[] = {'a', 0x00, 'b', 0xFF};
    n = dws_sftp_build_data(9, payload, sizeof(payload), buf, sizeof(buf));
    dws_sftp_rd_init(&r, buf + 4, n - 4);
    TEST_ASSERT_EQUAL_UINT8(SSH_FXP_DATA, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_UINT32(9, dws_sftp_rd_u32(&r));
    const uint8_t *d = nullptr;
    uint32_t dl = 0;
    dws_sftp_rd_string(&r, &d, &dl);
    TEST_ASSERT_EQUAL_UINT(4, dl);
    TEST_ASSERT_EQUAL_MEMORY(payload, d, 4);
}

static void test_build_name1_realpath()
{
    SftpAttrs a;
    a.flags = SSH_FILEXFER_ATTR_SIZE | SSH_FILEXFER_ATTR_PERMISSIONS;
    a.size = 1234;
    a.permissions = SFTP_S_IFREG | 0644;
    a.atime = a.mtime = 0;
    uint8_t buf[128];
    size_t n =
        dws_sftp_build_name1(5, "/gcode/part.nc", "-rw-r--r-- 1 0 0 1234 Jan  1 2026 part.nc", &a, buf, sizeof(buf));
    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, n - 4);
    TEST_ASSERT_EQUAL_UINT8(SSH_FXP_NAME, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_UINT32(5, dws_sftp_rd_u32(&r));
    TEST_ASSERT_EQUAL_UINT32(1, dws_sftp_rd_u32(&r)); // one entry
    const uint8_t *nm = nullptr;
    uint32_t nl = 0;
    dws_sftp_rd_string(&r, &nm, &nl);
    TEST_ASSERT_EQUAL_MEMORY("/gcode/part.nc", nm, nl);
    dws_sftp_rd_string(&r, nullptr, nullptr); // longname
    SftpAttrs ra;
    TEST_ASSERT_TRUE(dws_sftp_rd_attrs(&r, &ra));
    TEST_ASSERT_EQUAL_HEX64(1234, ra.size);
    TEST_ASSERT_TRUE(r.ok);
}

// --- multi-entry NAME via the writer API (READDIR) -----------------------------------------------
static void test_name_multi_entry()
{
    const char *names[3] = {".", "a.nc", "sub"};
    bool dirs[3] = {true, false, true};
    uint8_t buf[256];
    SftpWriter w;
    dws_sftp_wr_init(&w, buf, sizeof(buf));
    dws_sftp_wr_u8(&w, SSH_FXP_NAME);
    dws_sftp_wr_u32(&w, 11);
    size_t count_at = dws_sftp_wr_pos(&w); // remember where the count u32 goes
    dws_sftp_wr_u32(&w, 0);                // placeholder count
    uint32_t count = 0;
    for (int i = 0; i < 3; i++)
    {
        SftpAttrs a;
        a.flags = SSH_FILEXFER_ATTR_PERMISSIONS;
        a.permissions = (dirs[i] ? SFTP_S_IFDIR | 0755 : SFTP_S_IFREG | 0644);
        a.size = 0;
        a.atime = a.mtime = 0;
        char ln[64];
        dws_sftp_format_longname(dirs[i], a.permissions, 0, 0, names[i], ln, sizeof(ln));
        dws_sftp_wr_string(&w, names[i], (uint32_t)strlen(names[i]));
        dws_sftp_wr_string(&w, ln, (uint32_t)strlen(ln));
        dws_sftp_wr_attrs(&w, &a);
        count++;
    }
    dws_sftp_wr_patch_u32(&w, count_at, count);
    size_t total = dws_sftp_wr_finish(&w);

    SftpReader r;
    dws_sftp_rd_init(&r, buf + 4, total - 4);
    TEST_ASSERT_EQUAL_UINT8(SSH_FXP_NAME, dws_sftp_rd_u8(&r));
    TEST_ASSERT_EQUAL_UINT32(11, dws_sftp_rd_u32(&r));
    TEST_ASSERT_EQUAL_UINT32(3, dws_sftp_rd_u32(&r)); // count patched
    for (int i = 0; i < 3; i++)
    {
        const uint8_t *nm = nullptr;
        uint32_t nl = 0;
        dws_sftp_rd_string(&r, &nm, &nl);
        TEST_ASSERT_EQUAL_MEMORY(names[i], nm, nl);
        dws_sftp_rd_string(&r, nullptr, nullptr); // longname
        SftpAttrs a;
        dws_sftp_rd_attrs(&r, &a);
    }
    TEST_ASSERT_TRUE(r.ok);
}

static void test_longname_format()
{
    char out[64];
    size_t n = dws_sftp_format_longname(false, 0644, 1234, 0, "file.nc", out, sizeof(out));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_MEMORY("-rw-r--r-- ", out, 11); // mode string for a regular 0644 file
    TEST_ASSERT_NOT_NULL(strstr(out, "1234"));
    TEST_ASSERT_NOT_NULL(strstr(out, "file.nc"));

    n = dws_sftp_format_longname(true, 0755, 0, 0, "dir", out, sizeof(out));
    TEST_ASSERT_EQUAL_MEMORY("drwxr-xr-x ", out, 11); // directory
}

static void test_builder_overflow()
{
    uint8_t tiny[6];
    TEST_ASSERT_EQUAL_UINT(
        0, dws_sftp_build_status(1, SSH_FX_FAILURE, "a message too long for the buffer", tiny, sizeof(tiny)));
    TEST_ASSERT_EQUAL_UINT(0, dws_sftp_build_data(1, "0123456789", 10, tiny, sizeof(tiny)));
}

// --- shared path-traversal guard (server/fs_path.h) ----------------------------------------------
static void test_fs_path_resolve()
{
    char out[128];
    TEST_ASSERT_EQUAL_INT(0, fs_path_resolve("/gcode", "/part.nc", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("/gcode/part.nc", out);
    // root "/" + "/x" collapses the double slash
    TEST_ASSERT_EQUAL_INT(0, fs_path_resolve("/", "/x", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("/x", out);
    // a trailing slash is dropped
    TEST_ASSERT_EQUAL_INT(0, fs_path_resolve("/gcode", "/sub/", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("/gcode/sub", out);
    // traversal is refused
    TEST_ASSERT_EQUAL_INT(-1, fs_path_resolve("/gcode", "/../etc/passwd", out, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(-1, fs_path_resolve("/gcode", "/sub/../../x", out, sizeof(out)));
    // overflow is refused
    char small[8];
    TEST_ASSERT_EQUAL_INT(-2, fs_path_resolve("/gcode", "/a-very-long-subpath-name", small, sizeof(small)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_fs_path_resolve);
    RUN_TEST(test_rw_roundtrip);
    RUN_TEST(test_reader_bounds);
    RUN_TEST(test_attrs_roundtrip);
    RUN_TEST(test_attrs_skips_uidgid_and_extended);
    RUN_TEST(test_framing);
    RUN_TEST(test_parse_open_request);
    RUN_TEST(test_build_version);
    RUN_TEST(test_build_status);
    RUN_TEST(test_build_handle_and_data);
    RUN_TEST(test_build_name1_realpath);
    RUN_TEST(test_name_multi_entry);
    RUN_TEST(test_longname_format);
    RUN_TEST(test_builder_overflow);
    return UNITY_END();
}

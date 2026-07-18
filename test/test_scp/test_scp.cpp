// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host tests for services/scp: the SCP (RCP) wire codec. Covers parsing an `scp -t/-f <path>` exec command
// into its sink/source role + target path (with extra flags), parsing + building the `C<mode> <size> <name>`
// control line (octal mode, decimal size, name; malformed/D-record rejection), and the ack byte constants.

#include "services/scp/scp.h"
#include <stdint.h>
#include <string.h>
#include <unity.h>

void setUp()
{
}
void tearDown()
{
}

static void test_parse_cmd_sink()
{
    char path[128];
    ScpMode m = scp_parse_cmd("scp -t /gcode/prog.nc", strlen("scp -t /gcode/prog.nc"), path, sizeof(path));
    TEST_ASSERT_EQUAL(ScpMode::SINK, m);
    TEST_ASSERT_EQUAL_STRING("/gcode/prog.nc", path);
}

static void test_parse_cmd_source_with_flags()
{
    char path[128];
    ScpMode m = scp_parse_cmd("scp -v -p -f /a/b.txt", strlen("scp -v -p -f /a/b.txt"), path, sizeof(path));
    TEST_ASSERT_EQUAL(ScpMode::SOURCE, m);
    TEST_ASSERT_EQUAL_STRING("/a/b.txt", path);
}

static void test_parse_cmd_invalid()
{
    char path[128];
    // no -t/-f role
    TEST_ASSERT_EQUAL(ScpMode::INVALID, scp_parse_cmd("scp /x", strlen("scp /x"), path, sizeof(path)));
    // path too long for the buffer
    char small[4];
    TEST_ASSERT_EQUAL(ScpMode::INVALID,
                      scp_parse_cmd("scp -t /long/path", strlen("scp -t /long/path"), small, sizeof(small)));
}

static void test_parse_cline()
{
    uint32_t mode = 0;
    uint64_t size = 0;
    char name[64];
    TEST_ASSERT_TRUE(
        scp_parse_cline("C0644 60000 prog.nc\n", strlen("C0644 60000 prog.nc\n"), &mode, &size, name, sizeof(name)));
    TEST_ASSERT_EQUAL_HEX32(0644, mode);
    TEST_ASSERT_EQUAL_UINT64(60000, size);
    TEST_ASSERT_EQUAL_STRING("prog.nc", name);

    // no trailing newline is fine
    TEST_ASSERT_TRUE(scp_parse_cline("C0755 0 empty", strlen("C0755 0 empty"), &mode, &size, name, sizeof(name)));
    TEST_ASSERT_EQUAL_HEX32(0755, mode);
    TEST_ASSERT_EQUAL_UINT64(0, size);
    TEST_ASSERT_EQUAL_STRING("empty", name);
}

static void test_parse_cline_malformed()
{
    uint32_t mode = 0;
    uint64_t size = 0;
    char name[64];
    // a directory record (D) is not a file record
    TEST_ASSERT_FALSE(scp_parse_cline("D0755 0 dir\n", strlen("D0755 0 dir\n"), &mode, &size, name, sizeof(name)));
    // missing size field
    TEST_ASSERT_FALSE(scp_parse_cline("C0644 name\n", strlen("C0644 name\n"), &mode, &size, name, sizeof(name)));
    // empty name
    TEST_ASSERT_FALSE(scp_parse_cline("C0644 10 \n", strlen("C0644 10 \n"), &mode, &size, name, sizeof(name)));
    // non-octal mode digit (8)
    TEST_ASSERT_FALSE(scp_parse_cline("C0648 10 n\n", strlen("C0648 10 n\n"), &mode, &size, name, sizeof(name)));
}

static void test_build_cline_roundtrip()
{
    char out[64];
    size_t n = scp_build_cline(0644, 1234, "part.nc", out, sizeof(out));
    TEST_ASSERT_TRUE(n > 0);
    TEST_ASSERT_EQUAL_STRING("C0644 1234 part.nc\n", out);

    // parse it back
    uint32_t mode = 0;
    uint64_t size = 0;
    char name[32];
    TEST_ASSERT_TRUE(scp_parse_cline(out, n, &mode, &size, name, sizeof(name)));
    TEST_ASSERT_EQUAL_HEX32(0644, mode);
    TEST_ASSERT_EQUAL_UINT64(1234, size);
    TEST_ASSERT_EQUAL_STRING("part.nc", name);

    // overflow
    char tiny[6];
    TEST_ASSERT_EQUAL_UINT(0, scp_build_cline(0644, 1234, "part.nc", tiny, sizeof(tiny)));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_parse_cmd_sink);
    RUN_TEST(test_parse_cmd_source_with_flags);
    RUN_TEST(test_parse_cmd_invalid);
    RUN_TEST(test_parse_cline);
    RUN_TEST(test_parse_cline_malformed);
    RUN_TEST(test_build_cline_roundtrip);
    return UNITY_END();
}

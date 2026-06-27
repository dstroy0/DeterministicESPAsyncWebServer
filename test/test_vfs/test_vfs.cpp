// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Unit tests for the unified VFS (services/vfs) exercised through its built-in
// RAM backend: read/write/append/truncate, whole-file helpers, exists/size/
// remove/rename, and the bounded fail-closed paths (file-full, pool/handle
// exhaustion, unmounted, undersized read buffer). The same API drives the
// Arduino FS backend on hardware.

#include "services/vfs/vfs.h"
#include <stdio.h>
#include <string.h>
#include <unity.h>

void setUp()
{
    detws_vfs_mount(detws_vfs_ram());
    detws_vfs_ram_format();
}
void tearDown()
{
}

void test_write_then_read_file()
{
    const char *msg = "hello vfs";
    TEST_ASSERT_TRUE(detws_vfs_write_file("/a.txt", msg, strlen(msg)));
    TEST_ASSERT_TRUE(detws_vfs_exists("/a.txt"));
    TEST_ASSERT_EQUAL_INT32((long)strlen(msg), detws_vfs_size("/a.txt"));

    char buf[32];
    long n = detws_vfs_read_file("/a.txt", buf, sizeof(buf));
    TEST_ASSERT_EQUAL_INT32((long)strlen(msg), n);
    buf[n] = '\0';
    TEST_ASSERT_EQUAL_STRING(msg, buf);
}

void test_streamed_write_and_read()
{
    int h = detws_vfs_open("/s.bin", DETWS_VFS_WRITE);
    TEST_ASSERT_TRUE(h >= 0);
    TEST_ASSERT_EQUAL_INT(3, detws_vfs_write(h, "abc", 3));
    TEST_ASSERT_EQUAL_INT(3, detws_vfs_write(h, "def", 3));
    detws_vfs_close(h);
    TEST_ASSERT_EQUAL_INT32(6, detws_vfs_size("/s.bin"));

    h = detws_vfs_open("/s.bin", DETWS_VFS_READ);
    TEST_ASSERT_TRUE(h >= 0);
    char buf[8] = {0};
    TEST_ASSERT_EQUAL_INT(4, detws_vfs_read(h, buf, 4));
    TEST_ASSERT_EQUAL_STRING("abcd", buf);
    TEST_ASSERT_EQUAL_INT(2, detws_vfs_read(h, buf, 4)); // only 2 left
    TEST_ASSERT_EQUAL_INT(0, detws_vfs_read(h, buf, 4)); // EOF
    detws_vfs_close(h);
}

void test_write_mode_truncates()
{
    detws_vfs_write_file("/t.txt", "longer original", 15);
    detws_vfs_write_file("/t.txt", "short", 5);
    TEST_ASSERT_EQUAL_INT32(5, detws_vfs_size("/t.txt"));
    char buf[16];
    long n = detws_vfs_read_file("/t.txt", buf, sizeof(buf));
    buf[n] = '\0';
    TEST_ASSERT_EQUAL_STRING("short", buf);
}

void test_append_extends()
{
    detws_vfs_write_file("/log", "line1\n", 6);
    int h = detws_vfs_open("/log", DETWS_VFS_APPEND);
    TEST_ASSERT_TRUE(h >= 0);
    detws_vfs_write(h, "line2\n", 6);
    detws_vfs_close(h);
    TEST_ASSERT_EQUAL_INT32(12, detws_vfs_size("/log"));
    char buf[16];
    long n = detws_vfs_read_file("/log", buf, sizeof(buf));
    buf[n] = '\0';
    TEST_ASSERT_EQUAL_STRING("line1\nline2\n", buf);
}

void test_remove_and_rename()
{
    detws_vfs_write_file("/old", "data", 4);
    TEST_ASSERT_TRUE(detws_vfs_rename("/old", "/new"));
    TEST_ASSERT_FALSE(detws_vfs_exists("/old"));
    TEST_ASSERT_TRUE(detws_vfs_exists("/new"));
    char buf[8];
    long n = detws_vfs_read_file("/new", buf, sizeof(buf));
    buf[n] = '\0';
    TEST_ASSERT_EQUAL_STRING("data", buf);

    TEST_ASSERT_TRUE(detws_vfs_remove("/new"));
    TEST_ASSERT_FALSE(detws_vfs_exists("/new"));
    TEST_ASSERT_EQUAL_INT32(-1, detws_vfs_size("/new"));
}

void test_missing_file_fails_closed()
{
    TEST_ASSERT_FALSE(detws_vfs_exists("/nope"));
    TEST_ASSERT_EQUAL_INT32(-1, detws_vfs_size("/nope"));
    TEST_ASSERT_TRUE(detws_vfs_open("/nope", DETWS_VFS_READ) < 0);
    char buf[8];
    TEST_ASSERT_EQUAL_INT32(-1, detws_vfs_read_file("/nope", buf, sizeof(buf)));
    TEST_ASSERT_FALSE(detws_vfs_remove("/nope"));
    TEST_ASSERT_FALSE(detws_vfs_rename("/nope", "/x"));
}

void test_read_buffer_too_small_fails_closed()
{
    detws_vfs_write_file("/big", "0123456789", 10);
    char tiny[4];
    TEST_ASSERT_EQUAL_INT32(-1, detws_vfs_read_file("/big", tiny, sizeof(tiny)));
}

void test_file_full_is_bounded()
{
    int h = detws_vfs_open("/full", DETWS_VFS_WRITE);
    TEST_ASSERT_TRUE(h >= 0);
    static uint8_t chunk[256];
    memset(chunk, 'x', sizeof(chunk));
    size_t written = 0;
    for (int i = 0; i < 100; i++) // try to write far more than DETWS_VFS_RAM_FILE_SIZE
    {
        int w = detws_vfs_write(h, chunk, sizeof(chunk));
        if (w <= 0)
            break;
        written += (size_t)w;
    }
    detws_vfs_close(h);
    // Never exceeds the fixed per-file capacity (fail-closed, no overflow).
    TEST_ASSERT_EQUAL_INT32((long)DETWS_VFS_RAM_FILE_SIZE, detws_vfs_size("/full"));
    TEST_ASSERT_EQUAL_UINT32(DETWS_VFS_RAM_FILE_SIZE, (uint32_t)written);
}

void test_file_pool_exhaustion()
{
    char name[16];
    for (int i = 0; i < DETWS_VFS_RAM_FILES; i++)
    {
        snprintf(name, sizeof(name), "/f%d", i);
        TEST_ASSERT_TRUE(detws_vfs_write_file(name, "x", 1));
    }
    // One more distinct file must fail (pool full), not corrupt anything.
    TEST_ASSERT_FALSE(detws_vfs_write_file("/overflow", "x", 1));
}

void test_handle_pool_exhaustion()
{
    detws_vfs_write_file("/h", "data", 4);
    int handles[DETWS_VFS_MAX_OPEN];
    for (int i = 0; i < DETWS_VFS_MAX_OPEN; i++)
    {
        handles[i] = detws_vfs_open("/h", DETWS_VFS_READ);
        TEST_ASSERT_TRUE(handles[i] >= 0);
    }
    TEST_ASSERT_TRUE(detws_vfs_open("/h", DETWS_VFS_READ) < 0); // no handles left
    detws_vfs_close(handles[0]);
    TEST_ASSERT_TRUE(detws_vfs_open("/h", DETWS_VFS_READ) >= 0); // one freed
}

void test_unmounted_fails_closed()
{
    detws_vfs_mount(nullptr);
    TEST_ASSERT_TRUE(detws_vfs_open("/a", DETWS_VFS_READ) < 0);
    TEST_ASSERT_FALSE(detws_vfs_exists("/a"));
    TEST_ASSERT_EQUAL_INT32(-1, detws_vfs_size("/a"));
    TEST_ASSERT_FALSE(detws_vfs_write_file("/a", "x", 1));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_write_then_read_file);
    RUN_TEST(test_streamed_write_and_read);
    RUN_TEST(test_write_mode_truncates);
    RUN_TEST(test_append_extends);
    RUN_TEST(test_remove_and_rename);
    RUN_TEST(test_missing_file_fails_closed);
    RUN_TEST(test_read_buffer_too_small_fails_closed);
    RUN_TEST(test_file_full_is_bounded);
    RUN_TEST(test_file_pool_exhaustion);
    RUN_TEST(test_handle_pool_exhaustion);
    RUN_TEST(test_unmounted_fails_closed);
    return UNITY_END();
}

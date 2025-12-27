#include <stddef.h>   // size_t
#include <setjmp.h> 
#include <stdarg.h>  
#include <cmocka.h>
#include <string.h>
#include "../headers/sqlfs.h"

extern int setup_fs(void **);
extern int teardown_fs(void **);

static void test_write_read_simple(void **state)
{
    struct fuse_file_info fi = {0};

    fs_create("/file", 0644, &fi);

    const char *msg = "hello world";
    assert_int_equal(fs_write("/file", msg, strlen(msg), 0, &fi),
                     strlen(msg));

    char buf[64] = {0};
    assert_int_equal(fs_read("/file", buf, sizeof(buf), 0, &fi),
                     strlen(msg));

    assert_string_equal(buf, msg);
}

static void test_write_with_offset(void **state)
{
    struct fuse_file_info fi = {0};

    fs_create("/file", 0644, &fi);
    fs_write("/file", "abcdef", 6, 0, &fi);
    fs_write("/file", "XYZ", 3, 2, &fi);

    char buf[16] = {0};
    fs_read("/file", buf, sizeof(buf), 0, &fi);

    assert_string_equal(buf, "abXYZf");
}
static void test_write_zero_size(void **state)
{
    struct fuse_file_info fi = {0};

    fs_create("/file", 0644, &fi);
    assert_int_equal(fs_write("/file", "", 0, 0, &fi), 0);
}
static void test_write_noent(void **state)
{
    struct fuse_file_info fi = {0};
    assert_int_equal(fs_write("/nope", "x", 1, 0, &fi), -ENOENT);
}
static void test_write_isdir(void **state)
{
    struct fuse_file_info fi = {0};

    fs_mkdir("/dir", 0755);
    assert_int_equal(fs_write("/dir", "x", 1, 0, &fi), -EISDIR);
}
static void test_read_past_eof(void **state)
{
    struct fuse_file_info fi = {0};
    char buf[8] = {1};

    fs_create("/file", 0644, &fi);
    fs_write("/file", "abc", 3, 0, &fi);

    assert_int_equal(fs_read("/file", buf, sizeof(buf), 10, &fi), 0);
}
static void test_read_noent(void **state)
{
    struct fuse_file_info fi = {0};
    char buf[8];

    assert_int_equal(fs_read("/nope", buf, sizeof(buf), 0, &fi), -ENOENT);
}
static void test_read_isdir(void **state)
{
    struct fuse_file_info fi = {0};
    char buf[8];

    fs_mkdir("/dir", 0755);
    assert_int_equal(fs_read("/dir", buf, sizeof(buf), 0, &fi), -EISDIR);
}

void test_write_read_register(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_write_read_simple, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_write_with_offset, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_write_zero_size, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_write_noent, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_write_isdir, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_read_past_eof, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_read_noent, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_read_isdir, setup_fs, teardown_fs),
    };
    cmocka_run_group_tests(tests, NULL, NULL);
}


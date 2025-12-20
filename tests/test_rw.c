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

void test_write_read_register(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_write_read_simple, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_write_with_offset, setup_fs, teardown_fs)
    };
    cmocka_run_group_tests(tests, NULL, NULL);
}

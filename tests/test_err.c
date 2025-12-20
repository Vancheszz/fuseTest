#include <stddef.h>   // size_t
#include <setjmp.h>  
#include <stdarg.h> 
#include <cmocka.h>
#include "../headers/sqlfs.h"

extern int setup_fs(void **);
extern int teardown_fs(void **);

static void test_truncate_shrink(void **state)
{
    struct fuse_file_info fi = {0};

    fs_create("/file", 0644, &fi);
    fs_write("/file", "abcdef", 6, 0, &fi);

    assert_int_equal(fs_truncate("/file", 3, &fi), 0);

    char buf[8] = {0};
    assert_int_equal(fs_read("/file", buf, sizeof(buf), 0, &fi), 3);
    assert_memory_equal(buf, "abc", 3);
}

static void test_truncate_grow(void **state)
{
    struct fuse_file_info fi = {0};

    fs_create("/file", 0644, &fi);
    fs_truncate("/file", 10, &fi);

    char buf[16] = {1};
    fs_read("/file", buf, sizeof(buf), 0, &fi);

    for (int i = 0; i < 10; i++)
        assert_int_equal(buf[i], 0);
}

void test_errors_register(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_truncate_shrink, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_truncate_grow, setup_fs, teardown_fs)
    };
    cmocka_run_group_tests(tests, NULL, NULL);
}

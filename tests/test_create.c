#include <stddef.h>   // size_t
#include <setjmp.h>  
#include <stdarg.h> 
#include <cmocka.h>
#include "../headers/sqlfs.h"

extern int setup_fs(void **);
extern int teardown_fs(void **);

static void test_create_file(void **state)
{
    struct fuse_file_info fi = {0};

    assert_int_equal(fs_create("/file", 0644, &fi), 0);
    assert_int_equal(fs_create("/file", 0644, &fi), -EEXIST);
}

void test_create_register(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_create_file, setup_fs, teardown_fs)
    };
    cmocka_run_group_tests(tests, NULL, NULL);
}

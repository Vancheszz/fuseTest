#include <stddef.h>   // size_t
#include <setjmp.h>  
#include <stdarg.h> 
#include <cmocka.h>
#include "../headers/sqlfs.h"

extern int setup_fs(void **);
extern int teardown_fs(void **);

static void test_mkdir_rmdir(void **state)
{
    assert_int_equal(fs_mkdir("/dir", 0755), 0);
    assert_int_equal(fs_mkdir("/dir", 0755), -EEXIST);

    assert_int_equal(fs_rmdir("/dir"), 0);
    assert_int_equal(fs_rmdir("/dir"), -ENOENT);
}

static void test_rmdir_not_empty(void **state)
{
    struct fuse_file_info fi = {0};

    fs_mkdir("/dir", 0755);
    fs_create("/dir/file", 0644, &fi);

    assert_int_equal(fs_rmdir("/dir"), -ENOTEMPTY);
}

void test_dir_register(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_mkdir_rmdir, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_rmdir_not_empty, setup_fs, teardown_fs)
    };
    cmocka_run_group_tests(tests, NULL, NULL);
}

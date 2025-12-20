#include <stddef.h>   // size_t
#include <setjmp.h>  
#include <stdarg.h> 
#include <cmocka.h>
#include "../headers/sqlfs.h"

extern int setup_fs(void **);
extern int teardown_fs(void **);

static void test_getattr_file(void **state)
{
    struct fuse_file_info fi = {0};
    struct stat st;

    fs_create("/file", 0644, &fi);

    assert_int_equal(fs_getattr("/file", &st, &fi), 0);
    assert_true(S_ISREG(st.st_mode));
    assert_int_equal(st.st_size, 0);
}

static void test_getattr_dir(void **state)
{
    struct stat st;

    fs_mkdir("/dir", 0755);
    assert_int_equal(fs_getattr("/dir", &st, NULL), 0);
    assert_true(S_ISDIR(st.st_mode));
}

static void test_getattr_noent(void **state)
{
    struct stat st;
    assert_int_equal(fs_getattr("/nope", &st, NULL), -ENOENT);
}

void test_getattr_register(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_getattr_file, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_getattr_dir, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_getattr_noent, setup_fs, teardown_fs),
    };
    cmocka_run_group_tests(tests, NULL, NULL);
}

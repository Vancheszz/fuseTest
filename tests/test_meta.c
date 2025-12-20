#include <stddef.h>   // size_t
#include <setjmp.h>  
#include <stdarg.h> 
#include <cmocka.h>
#include "../headers/sqlfs.h"

extern int setup_fs(void **);
extern int teardown_fs(void **);

static void test_chmod(void **state)
{
    struct fuse_file_info fi = {0};
    struct stat st;

    fs_create("/file", 0644, &fi);
    assert_int_equal(fs_chmod("/file", 0600, &fi), 0);

    fs_getattr("/file", &st, &fi);
    assert_int_equal(st.st_mode & 0777, 0600);
}

static void test_chown(void **state)
{
    struct fuse_file_info fi = {0};
    struct stat st;

    fs_create("/file", 0644, &fi);
    assert_int_equal(fs_chown("/file", 123, 456, &fi), 0);

    fs_getattr("/file", &st, &fi);
    assert_int_equal(st.st_uid, 123);
    assert_int_equal(st.st_gid, 456);
}

void test_meta_register(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(
            test_chmod, setup_fs, teardown_fs),
        cmocka_unit_test_setup_teardown(
            test_chown, setup_fs, teardown_fs),
    };
    cmocka_run_group_tests(tests, NULL, NULL);
}
static void test_utimens(void **state)
{
    struct fuse_file_info fi = {0};
    struct stat st;
    struct timespec ts[2];

    fs_create("/file", 0644, &fi);

    ts[0].tv_sec = 100;
    ts[1].tv_sec = 200;

    assert_int_equal(fs_utimens("/file", ts, &fi), 0);

    fs_getattr("/file", &st, &fi);
    assert_int_equal(st.st_atime, 100);
    assert_int_equal(st.st_mtime, 200);
}

static void test_rename(void **state)
{
    struct fuse_file_info fi = {0};

    fs_create("/file", 0644, &fi);
    assert_int_equal(fs_rename("/file", "/newfile", 0), 0);

    char buf[8];
    assert_int_equal(fs_read("/newfile", buf, sizeof(buf), 0, &fi), 0);
    assert_int_equal(fs_read("/file", buf, sizeof(buf), 0, &fi), -ENOENT);
}

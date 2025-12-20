#include <stddef.h>   // size_t
#include <setjmp.h>  
#include <stdarg.h> 
#include <cmocka.h>

void test_create_register(void);
void test_dir_register(void);
void test_write_read_register(void);
void test_truncate_register(void);
void test_errors_register(void);
void test_getattr_register(void);
void test_meta_register(void);

int main(void)
{
    test_getattr_register();
    test_meta_register();
    test_create_register();
    test_dir_register();
    test_write_read_register();
    test_truncate_register();
    test_errors_register();
    return 0;
}

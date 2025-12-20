#include <stddef.h>   // size_t
#include <setjmp.h>  
#include <stdarg.h> 
#include <cmocka.h>
#include "../headers/sqlfs.h"   
#include "../headers/db.h"     // где объявлен init_db

int setup_fs(void **state)
{
    (void)state;
    init_db(":memory:");   // SQLite в памяти
    return 0;
}

int teardown_fs(void **state)
{
    (void)state;
    sqlite3_close(db);
    return 0;
}

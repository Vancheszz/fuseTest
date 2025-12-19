#include "../headers/sqlfs.h"
#include "../headers/db.h"
sqlite3 *db; 
// db init 
void init_db(const char *db_path){
    if(sqlite3_open(db_path, &db) != SQLITE_OK){
        fprintf(stderr, "Cannot open DB: %s!!!\n", sqlite3_errmsg(db));
        exit(1);
    }
    //set up nodes
    const char *sql_node =
    "CREATE TABLE IF NOT EXISTS nodes ("
        " path TEXT PRIMARY KEY,"
        " mode INTEGER,"
        " uid INTEGER,"
        " gid INTEGER,"
        " size INTEGER,"
        " atime INTEGER,"
        " mtime INTEGER,"
        " ctime INTEGER"
        ");";
    //set up data 
    const char *sql_data = 
    "CREATE TABLE IF NOT EXISTS data("
    "path TEXT,"
    "chunk INTEGER,"
    "content BLOB,"
    "PRIMARY KEY (path,chunk)"
    ");";

        char *errmsg = NULL;
        if (sqlite3_exec(db,sql_node, NULL, NULL, &errmsg)!= SQLITE_OK){
            fprintf(stderr, "SQL ERROR %s!!!\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
            exit(1);
        }

        if (sqlite3_exec(db,sql_data, NULL, NULL, &errmsg)!= SQLITE_OK){
            fprintf(stderr, "SQL ERROR %s!!!\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
            exit(1);
        }
        const char *root_sql = 
        "INSERT OR IGNORE INTO nodes "
        "(path, mode, uid, gid, size, atime, mtime, ctime) "
        "VALUES ('/', ?, ?, ?, 0, ?, ?, ?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, root_sql, -1, &stmt, NULL);

        time_t now = time(NULL);
        sqlite3_bind_int(stmt, 1, __S_IFDIR | 0755);
        sqlite3_bind_int(stmt, 2, getuid());
        sqlite3_bind_int(stmt, 3, getgid());
        sqlite3_bind_int(stmt, 4, now);
        sqlite3_bind_int(stmt, 5, now);
        sqlite3_bind_int(stmt, 6, now);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

}

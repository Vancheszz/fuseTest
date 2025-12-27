#include "../headers/sqlfs.h"
void split_path(const char *path, char * parent, char *name){
    char *splash = strrchr(path, '/');

    if (splash == path){
        strcpy(parent,"/");
        strcpy(name, path+1);
    }else{
        strncpy(parent,path,splash-path); 
        parent[splash - path] = '\0'; 
        strcpy(name, splash+1);
    } 

}
int is_directory(const char *path)
{
    const char *sql =
        "SELECT type FROM nodes WHERE path = ?;";

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    int is_dir = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int type = sqlite3_column_int(stmt, 0);
        is_dir = (type == S_IFDIR);
    }

    sqlite3_finalize(stmt);
    return is_dir;
}

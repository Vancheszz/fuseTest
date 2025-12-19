#include "../headers/sqlfs.h"
int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi ){
    (void) fi; 
    memset(stbuf, 0, sizeof(struct stat)); 

    const char *sql = 
        "SELECT mode, uid, gid, size, atime, mtime, ctime "
        "FROM nodes WHERE path = ?;";
    sqlite3_stmt *stmt; 
    sqlite3_prepare_v2(db,sql, -1, &stmt,NULL);
    sqlite3_bind_text(stmt, 1, path,-1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt); 

    //no file
    if (rc != SQLITE_ROW){
        sqlite3_finalize(stmt);
        return -ENOENT;
    }

    //set values in stat buffer 
    stbuf->st_mode  = sqlite3_column_int(stmt, 0);
    stbuf->st_uid   = sqlite3_column_int(stmt, 1);
    stbuf->st_gid   = sqlite3_column_int(stmt, 2);
    stbuf->st_size  = sqlite3_column_int(stmt, 3);
    stbuf->st_atime = sqlite3_column_int(stmt, 4);
    stbuf->st_mtime = sqlite3_column_int(stmt, 5);
    stbuf->st_ctime = sqlite3_column_int(stmt, 6);
    stbuf->st_nlink = S_ISDIR(stbuf->st_mode) ? 2 : 1;

    sqlite3_finalize(stmt);
    return 0;
    
}
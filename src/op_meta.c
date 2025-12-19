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
int fs_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) fi;
    sqlite3_stmt *stmt;
    const char *sql;

    sql = "SELECT mode FROM nodes WHERE path = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -EIO;
    }
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }

    mode_t old_mode = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

  
    mode_t new_mode = (old_mode & S_IFMT) | (mode & ~S_IFMT);

    sql = "UPDATE nodes SET mode = ?, ctime = ? WHERE path = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -EIO;
    }

    sqlite3_bind_int(stmt, 1, new_mode);
    sqlite3_bind_int64(stmt, 2, (long long)time(NULL)); 
    sqlite3_bind_text(stmt, 3, path, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return -EIO;
    }

    return 0;
}
int fs_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    (void) fi;
    sqlite3_stmt *stmt;
    const char *sql;

    if (uid == (uid_t)-1 || gid == (gid_t)-1) {
        sql = "SELECT uid, gid FROM nodes WHERE path = ?;";
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (uid == (uid_t)-1) uid = sqlite3_column_int(stmt, 0);
            if (gid == (gid_t)-1) gid = sqlite3_column_int(stmt, 1);
        } else {
            sqlite3_finalize(stmt);
            return -ENOENT;
        }
        sqlite3_finalize(stmt);
    }

    sql = "UPDATE nodes SET uid = ?, gid = ?, ctime = ? WHERE path = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -EIO;
    }

    sqlite3_bind_int(stmt, 1, uid);
    sqlite3_bind_int(stmt, 2, gid);
    sqlite3_bind_int64(stmt, 3, (long long)time(NULL));
    sqlite3_bind_text(stmt, 4, path, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return (rc == SQLITE_DONE) ? 0 : -EIO;
}
int fs_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
    (void) fi;
    sqlite3_stmt *stmt;
    const char *sql;

    sql = "UPDATE nodes SET atime = ?, mtime = ?, ctime = ? WHERE path = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -EIO;
    }

    sqlite3_bind_double(stmt, 1, (double)tv[0].tv_sec);
    sqlite3_bind_double(stmt, 2, (double)tv[1].tv_sec);
    
    sqlite3_bind_double(stmt, 3, (double)time(NULL));
    
    sqlite3_bind_text(stmt, 4, path, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return -EIO;
    }

    return 0;
}
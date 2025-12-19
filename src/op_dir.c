#include "../headers/sqlfs.h"
int fs_mkdir(const char *path, mode_t mode) {
    char parent[512], name[256]; 
    split_path(path, parent, name); 
    sqlite3_stmt *stmt; 
    const char* sql;

    sql = "SELECT mode FROM nodes WHERE path = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -EIO;
    }
    
    sqlite3_bind_text(stmt, 1, parent, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT; 
    }

    int parent_mode = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (!S_ISDIR(parent_mode)) {
        return -ENOTDIR; 
    }

    sql = "SELECT 1 FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -EEXIST; // Такая папка уже есть
    }
    sqlite3_finalize(stmt);

   
    sql = "INSERT INTO nodes (path, mode, uid, gid, size, atime, mtime, ctime) "
          "VALUES (?, ?, ?, ?, 0, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return -EIO;
    }

    time_t now = time(NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, S_IFDIR | mode);
    sqlite3_bind_int(stmt, 3, getuid());
    sqlite3_bind_int(stmt, 4, getgid());
    sqlite3_bind_int64(stmt, 5, (long long)now);
    sqlite3_bind_int64(stmt, 6, (long long)now);
    sqlite3_bind_int64(stmt, 7, (long long)now);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return -EIO;
    }

    return 0; 
}
//rmdir 
int fs_rmdir(const char *path){
   
    sqlite3_stmt *stmt;
    const char *sql;

   
    if (strcmp(path, "/") == 0){
        return -EBUSY;
    }

    sql = "SELECT mode FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }

    int mode = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (!S_ISDIR(mode)) {
        return -ENOTDIR;
    }

    sql = "SELECT 1 FROM nodes "
        "WHERE path LIKE ? || '/%';";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOTEMPTY;
    }
    sqlite3_finalize(stmt);

    sql = "DELETE FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return 0;

}

int fs_readdir(const char *path, void *buf, fuse_fill_dir_t fill_dir, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags){
    (void) fi; 
    (void) offset; 
    (void) flags;
    fill_dir(buf, ".", NULL, 0, 0);
    fill_dir(buf, "..", NULL, 0, 0);

    const char *sql =
        "SELECT path FROM nodes WHERE path != '/' AND "
        "substr(path, 1, length(?)) = ?;";

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *full = (const char *)sqlite3_column_text(stmt, 0);

        const char *name = strrchr(full, '/');
        if (name && strlen(name) > 1) {
            fill_dir(buf, name + 1, NULL, 0, 0);
        }
    }

    sqlite3_finalize(stmt);
    return 0; 

}
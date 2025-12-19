#include "../headers/sqlfs.h"
int fs_unlink(const char* path){
    sqlite3_stmt *stmt;
    const char *sql;

    //file exist
    sql = "SELECT mode FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }

    int mode = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    //dir error 
    if (S_ISDIR(mode)) {
        return -EISDIR;
    }
    //unlink in DB 
    sql = "DELETE FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sql = "DELETE FROM data WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return 0;

}
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi){
    (void) fi; 
    char parent[512], name[256]; 
    split_path(path, parent, name); 
    sqlite3_stmt *stmt; 
    const char *sql; 

    if (strcmp(path, "/") == 0){
        return -EEXIST;
    }

    sql = "SELECT mode FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
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
        return -EEXIST;
    }
    sqlite3_finalize(stmt);

    sql =
        "INSERT INTO nodes "
        "(path, mode, uid, gid, size, atime, mtime, ctime) "
        "VALUES (?, ?, ?, ?, 0, ?, ?, ?);";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    time_t now = time(NULL);

    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, __S_IFREG | mode);
    sqlite3_bind_int(stmt, 3, getuid());
    sqlite3_bind_int(stmt, 4, getgid());
    sqlite3_bind_int(stmt, 5, now);
    sqlite3_bind_int(stmt, 6, now);
    sqlite3_bind_int(stmt, 7, now);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return 0; 


}


int fs_read(const char* path, char *buf,  size_t size, off_t offset, struct fuse_file_info *fi){
    (void) fi; 
    //sql 
    sqlite3_stmt *stmt; 
    const char *sql; 

    //file exist
    sql = "SELECT size, mode FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }
    off_t file_size = sqlite3_column_int64(stmt, 0);
    int mode = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    if (S_ISDIR(mode))
        return -EISDIR;

    
    if (offset >= file_size)
        return 0;

    if (offset + size > file_size)
        size = file_size - offset;

    size_t bytes_read = 0;

    int first_chunk = offset / CHUNK_SIZE;
    int last_chunk  = (offset + size - 1) / CHUNK_SIZE;

    //read chunk 

    for (int chunk = first_chunk; chunk <= last_chunk; chunk++) {

        size_t chunk_start = chunk * CHUNK_SIZE;
        size_t chunk_end   = chunk_start + CHUNK_SIZE;

        size_t copy_start = offset > chunk_start
                            ? offset - chunk_start
                            : 0;

        size_t copy_end = (offset + size < chunk_end)
                          ? offset + size - chunk_start
                          : CHUNK_SIZE;

        size_t to_copy = copy_end - copy_start;
    sql ="SELECT content FROM data "
        "WHERE path = ? AND chunk = ?;";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, chunk);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const void *blob = sqlite3_column_blob(stmt, 0);
        int blob_size = sqlite3_column_bytes(stmt, 0);
        if (copy_start < (size_t)blob_size) {
            size_t available = blob_size - copy_start;
            if (available < to_copy)
                to_copy = available;
            memcpy(buf + bytes_read,(const char *)blob + copy_start,to_copy);
            }
    }else {
        memset(buf + bytes_read, 0, to_copy);
    }
        sqlite3_finalize(stmt);
        bytes_read += to_copy;
    }
    sql = "UPDATE nodes SET atime = ? WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, time(NULL));
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return bytes_read;



}

//write 
int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    (void) fi;

    sqlite3_stmt *stmt;
    const char *sql;

    //check file 
    sql = "SELECT size, mode FROM nodes WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }

    off_t old_size = sqlite3_column_int64(stmt, 0);
    int mode = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    if (S_ISDIR(mode))
        return -EISDIR;
    //chunk calculate 
    int first_chunk = offset / CHUNK_SIZE;
    int last_chunk  = (offset + size - 1) / CHUNK_SIZE;

    size_t written = 0;
     for (int chunk = first_chunk; chunk <= last_chunk; chunk++) {

        unsigned char chunk_buf[CHUNK_SIZE];
        memset(chunk_buf, 0, CHUNK_SIZE);

        //load chunks 
        sql =
            "SELECT content FROM data "
            "WHERE path = ? AND chunk = ?;";

        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, chunk);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const void *blob = sqlite3_column_blob(stmt, 0);
            int blob_size = sqlite3_column_bytes(stmt, 0);
            memcpy(chunk_buf, blob, blob_size);
        }
        sqlite3_finalize(stmt);
        size_t chunk_start = chunk * CHUNK_SIZE;
        size_t write_start = offset > chunk_start
                             ? offset - chunk_start
                             : 0;

        size_t write_end = (offset + size < chunk_start + CHUNK_SIZE)
                           ? offset + size - chunk_start
                           : CHUNK_SIZE;

        size_t to_write = write_end - write_start;
        memcpy(chunk_buf + write_start, buf + written, to_write);
        //save chunk
        sql =
            "INSERT OR REPLACE INTO data "
            "(path, chunk, content) VALUES (?, ?, ?);";

        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, chunk);
        sqlite3_bind_blob(stmt, 3, chunk_buf, CHUNK_SIZE, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        written += to_write;
    }
     off_t new_size = offset + size > old_size
                     ? offset + size
                     : old_size;

    sql =
        "UPDATE nodes "
        "SET size = ?, mtime = ?, ctime = ? "
        "WHERE path = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int64(stmt, 1, new_size);
    sqlite3_bind_int(stmt, 2, time(NULL));
    sqlite3_bind_int(stmt, 3, time(NULL));
    sqlite3_bind_text(stmt, 4, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return size; 

}
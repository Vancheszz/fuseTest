#pragma once

#define FUSE_USE_VERSION 31

#include <stdio.h>
#include <fuse3/fuse.h>
#include <sqlite3.h> //sqllite moment
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define CHUNK_SIZE 4096

extern sqlite3 *db;


void split_path(const char *path, char *parent, char *name);
int is_directory(const char *path);
int fs_mkdir(const char *path, mode_t mode);
int fs_rmdir(const char *path);
int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi,
                  enum fuse_readdir_flags flags);


int fs_create(const char *path, mode_t mode,
                 struct fuse_file_info *fi);
int fs_unlink(const char *path);
int fs_read(const char *path, char *buf,
               size_t size, off_t offset,
               struct fuse_file_info *fi);
int fs_write(const char *path, const char *buf,
                size_t size, off_t offset,
                struct fuse_file_info *fi);


int fs_getattr(const char *path, struct stat *stbuf,
                  struct fuse_file_info *fi);
int fs_rename(const char *from, const char *to, unsigned int flags);

int fs_truncate(const char *path, off_t size, struct fuse_file_info *fi);
int fs_chmod(const char *path, mode_t mode, struct fuse_file_info *fi);
int fs_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi);
int fs_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi);
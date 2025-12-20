
#include "../headers/sqlfs.h"
#include "../headers/db.h"

#ifndef UNIT_TEST
static const struct fuse_operations fs_oper =  {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .mkdir = fs_mkdir,
    .create = fs_create, 
    .unlink = fs_unlink, 
    .rmdir = fs_rmdir,
    .read = fs_read, 
    .write = fs_write,
    .rename = fs_rename,
    .chown = fs_chown,
    .chmod = fs_chmod,
    .utimens = fs_utimens

};

int main(int argc, char ** argv){
    init_db("fs.db");
    return fuse_main(argc, argv, &fs_oper, NULL);
  
}
#endif
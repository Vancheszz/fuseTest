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
#ifndef FS_H
#define FS_H

#include <stdint.h>

#define MAX_FILES     16
#define MAX_FILENAME  32
#define MAX_FILESIZE  512

struct file {
    char     name[MAX_FILENAME];
    uint32_t size;
    char     data[MAX_FILESIZE];
    uint8_t  used;
};

void     fs_init();
int      fs_create(const char* name, const char* data);
struct file* fs_open(const char* name);
void     fs_list();
int      fs_delete(const char* name);

#endif
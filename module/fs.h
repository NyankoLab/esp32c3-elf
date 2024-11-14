#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void fs_init(void);
int fs_stat(const char* name);
int fs_open(const char* name, const char* mode);
void fs_close(int fd);

int fs_getc(int fd);
char* fs_gets(char* buffer, int length, int fd);

void fs_seek(int pos, int fd);
int fs_tell(int fd);

int fs_read(void* buffer, int length, int fd);
int fs_write(const void* buffer, int length, int fd);

int fs_mkdir(const char* name);

int fs_remove(const char* name);

int fs_dir_open(const char* name);
int fs_dir_read(int dir, char* name, int length, int* size);
void fs_dir_close(int dir);

#ifdef __cplusplus
}
#endif

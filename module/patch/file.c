#include "esp32c3.h"
#include <sys/stat.h>
#include "module/fs.h"

int __real_fclose(FILE* file);
int __wrap_fclose(FILE* file)
{
    if (file == stdin || file == stdout || file == stderr)
        return __real_fclose(file);
    fs_close((int)file);
    return 0;
}

size_t __real_fread(void* buffer, size_t size, size_t n, FILE* file);
size_t __wrap_fread(void* buffer, size_t size, size_t n, FILE* file)
{
    if (file == stdin || file == stdout || file == stderr)
        return __real_fread(buffer, size, n, file);
    size_t length = fs_read(buffer, size * n, (int)file);
    return length / size;
}

size_t __real_fwrite(const void* buffer, size_t size, size_t n, FILE * file);
size_t __wrap_fwrite(const void* buffer, size_t size, size_t n, FILE * file)
{
    if (file == stdin || file == stdout || file == stderr)
        return __real_fwrite(buffer, size, n, file);
    size_t length = fs_write(buffer, size * n, (int)file);
    return length / size;
}

int __real_fseek(FILE* file, long pos, int type);
int __wrap_fseek(FILE* file, long pos, int type)
{
    if (file == stdin || file == stdout || file == stderr)
        return __real_fseek(file, pos, type);
    fs_seek((int)file, pos, type);
    return 0;
}

long __real_ftell(FILE* file);
long __wrap_ftell(FILE* file)
{
    if (file == stdin || file == stdout || file == stderr)
        return __real_ftell(file);
    return fs_tell((int)file);
}

FILE* __wrap_fopen(const char* name, const char* type)
{
    int fd = fs_open(name, type);
    if (fd <= 0)
        return NULL;
    return (FILE*)fd;
}

int __wrap_stat(const char* name, struct stat* stat)
{
    memset(stat, 0, sizeof(struct stat));
    stat->st_size = fs_stat(name);
    return stat->st_size >= 0 ? 0 : -1;
}

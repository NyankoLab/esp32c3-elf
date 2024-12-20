#include "esp32c3.h"
#include <esp_flash.h>
#include <spi_flash_mmap.h>
#include "littlefs/lfs.h"

#define IROM_ADDR   0x42000000
#define PHYS_BEGIN  0x42020000
#define PHYS_END    0x42100000
#define PHYS_ADDR   (PHYS_BEGIN - IROM_ADDR)
#define PHYS_SIZE   (PHYS_END - PHYS_BEGIN)
#define PHYS_PAGE   0x100
#define PHYS_BLOCK  SPI_FLASH_SEC_SIZE

#define TAG __FILE_NAME__

static int32_t fs_hal_read(uint32_t addr, uint32_t size, uint8_t* dst)
{
    uint32_t buffer[PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < PHYS_PAGE ? size : PHYS_PAGE;
        int length_aligned = (length + 3) & ~3;
        esp_err_t result = esp_flash_read(NULL, buffer, addr, length_aligned);
        if (result != ESP_OK)
        {
            ESP_LOGE(TAG, "%s(NULL, %p, %p, %d): %d", "esp_flash_read", (char*)buffer, (char*)addr, length_aligned, result);
            return -1;
        }
        memcpy(dst, buffer, length);
        addr += length;
        size -= length;
    }
    return 0;
}

static int32_t fs_hal_write(uint32_t addr, uint32_t size, uint8_t* src)
{
    uint32_t buffer[PHYS_PAGE / sizeof(uint32_t)];
    while (size)
    {
        int length = size < PHYS_PAGE ? size : PHYS_PAGE;
        int length_aligned = (length + 3) & ~3;
        memcpy(buffer, src, length);
        esp_err_t result = esp_flash_write(NULL, buffer, addr, length_aligned);
        if (result != ESP_OK)
        {
            ESP_LOGE(TAG, "%s(NULL, %p, %p, %d): %d", "spi_flash_write", (char*)buffer, (char*)addr, length_aligned, result);
            return -1;
        }
        addr += length;
        size -= length;
    }
    return 0;
}

static int32_t fs_hal_erase(uint32_t addr, uint32_t size)
{
    esp_err_t result = esp_flash_erase_region(NULL, addr, size);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "%s(NULL, %p, %d): %d", "esp_flash_erase_region", (char*)addr, size, result);
        return -1;
    }
    return 0;
}

static lfs_t fs IRAM_BSS_ATTR;

static int lfs_flash_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* dst, lfs_size_t size)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK) + off;
    return fs_hal_read(addr, size, (uint8_t*)dst);
}

static int lfs_flash_prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK) + off;
    return fs_hal_write(addr, size, (uint8_t*)buffer);
}

static int lfs_flash_erase(const struct lfs_config* c, lfs_block_t block)
{
    uint32_t addr = PHYS_BEGIN - IROM_ADDR + (block * PHYS_BLOCK);
    uint32_t size = PHYS_BLOCK;
    return fs_hal_erase(addr, size);
}

static int lfs_flash_sync(const struct lfs_config* c)
{
    return 0;
}

void fs_init(void)
{
    static struct lfs_config cfg IRAM_BSS_ATTR;
    cfg.read = lfs_flash_read;
    cfg.prog = lfs_flash_prog;
    cfg.erase = lfs_flash_erase;
    cfg.sync = lfs_flash_sync;
    cfg.read_size = 64;
    cfg.prog_size = 64;
    cfg.block_size = PHYS_BLOCK;
    cfg.block_count = PHYS_SIZE / PHYS_BLOCK;
    cfg.block_cycles = 16;
    cfg.cache_size = 64;
    cfg.lookahead_size = 64;

    if (lfs_mount(&fs, &cfg) != LFS_ERR_OK)
    {
        lfs_format(&fs, &cfg);
        lfs_mount(&fs, &cfg);
    }
}

int fs_stat(const char* name)
{
    struct lfs_info info = { 0, -1 };
    lfs_stat(&fs, name, &info);
    return info.size;
}

int fs_open(const char* name, const char* mode)
{
    int flags = 0;
    switch (mode[0])
    {
    case 'a':
        flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND;
        break;
    case 'r':
        flags = LFS_O_RDONLY;
        break;
    case 'w':
        flags = LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC;
        break;
    default:
        break;
    }
    char* temp = strdup(name);
    lfs_file_t* fd = calloc(1, sizeof(lfs_file_t));
    int result = lfs_file_open(&fs, fd, temp, flags);
    if (result != LFS_ERR_OK)
    {
        free(fd);
        fd = (lfs_file_t*)result;
    }
    free(temp);
    return (int)fd;
}

void fs_close(int fd)
{
    if (fd <= 0)
        return;
    lfs_file_close(&fs, (lfs_file_t*)fd);
    free((lfs_file_t*)fd);
}

int fs_getc(int fd)
{
    if (fd <= 0)
        return -1;
    uint8_t c;
    if (lfs_file_read(&fs, (lfs_file_t*)fd, &c, 1) != 1)
        return -1;
    return c;
}

char* fs_gets(char* buffer, int length, int fd)
{
    if (fd <= 0)
        return NULL;

    int pos = lfs_file_tell(&fs, (lfs_file_t*)fd);
    length = lfs_file_read(&fs, (lfs_file_t*)fd, buffer, length - 1);
    buffer[length] = 0;

    for (int i = 0; i < length; ++i)
    {
        char c = buffer[i];
        if (c == 0 || c == '\r' || c == '\n')
        {
            buffer[i] = '\0';
            if (c == '\r' && (i + 1) < length)
            {
                c = buffer[i + 1];
                if (c == '\n')
                {
                    ++i;
                }
            }
            lfs_file_seek(&fs, (lfs_file_t*)fd, pos + i + 1, LFS_SEEK_SET);
            break;
        }
    }

    return buffer;
}

void fs_seek(int fd, int pos, int type)
{
    if (fd <= 0)
        return;
    lfs_file_seek(&fs, (lfs_file_t*)fd, pos, type);
}

int fs_tell(int fd)
{
    if (fd <= 0)
        return 0;
    return lfs_file_tell(&fs, (lfs_file_t*)fd);
}

int fs_read(void* buffer, int length, int fd)
{
    if (fd <= 0)
        return 0;
    return lfs_file_read(&fs, (lfs_file_t*)fd, buffer, length);
}

int fs_write(const void* buffer, int length, int fd)
{
    if (fd <= 0)
        return 0;
    return lfs_file_write(&fs, (lfs_file_t*)fd, (void*)buffer, length);
}

int fs_mkdir(const char* name)
{
    return lfs_mkdir(&fs, name);
}

int fs_remove(const char* name)
{
    lfs_dir_t dir = {};
    if (lfs_dir_open(&fs, &dir, name) == 0)
    {
        lfs_dir_seek(&fs, &dir, 2);

        struct lfs_info info;
        while (lfs_dir_read(&fs, &dir, &info) > 0)
        {
            size_t length = strlen(name) + 1 + strlen(info.name) + 1;
            char* temp = malloc(length);
            strcpy(temp, name);
            strcat(temp, "/");
            strcat(temp, info.name);
            fs_remove(temp);
            free(temp);
        }
        lfs_dir_close(&fs, &dir);
    }
    return lfs_remove(&fs, name);
}

int fs_dir_open(const char* name)
{
    lfs_dir_t* dir = calloc(1, sizeof(lfs_dir_t));
    if (lfs_dir_open(&fs, dir, name) == 0)
    {
        lfs_dir_seek(&fs, dir, 2);
        return (int)dir;
    }
    free(dir);
    return -1;
}

int fs_dir_read(int dir, char* name, int length, int* size)
{
    if (dir < 0)
        return -1;
    struct lfs_info info;
    if (lfs_dir_read(&fs, (lfs_dir_t*)dir, &info) <= 0)
        return -1;
    if (name)
        strncpy(name, info.name, length);
    if (size)
        *size = info.size;
    return 0;
}

void fs_dir_close(int dir)
{
    if (dir < 0)
        return;
    lfs_dir_close(&fs, (lfs_dir_t*)dir);
    free((lfs_dir_t*)dir);
}

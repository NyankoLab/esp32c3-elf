#include "esp32c3.h"
#include "module/fs.h"

#define HAP_SUCCESS 0
#define HAP_FAIL    -1
#define TAG         __FILE_NAME__

httpd_handle_t* hap_httpd_get_handle();
int __real_hap_httpd_start(void);
int __wrap_hap_httpd_start(void)
{
    httpd_handle_t* handle = hap_httpd_get_handle();
    if (handle) {
        extern httpd_handle_t httpd_server;
        (*handle) = httpd_server;
        return 0;
    }
    return __real_hap_httpd_start();
}

int hap_keystore_init(void)
{
    return HAP_SUCCESS;
}

int hap_keystore_get(const char* name_space, const char* key, uint8_t* val, size_t* val_size)
{
    char path[64];
    snprintf(path, 64, "%s/%s/%s", "hap", name_space, key);
    int fd = fs_open(path, "rb");
    if (fd < 0)
        return HAP_FAIL;
    *val_size = fs_read(val, *val_size, fd);
    fs_close(fd);
    return HAP_SUCCESS;
}

int hap_factory_keystore_get(const char* name_space, const char* key, uint8_t* val, size_t* val_size)
{
    return hap_keystore_get(name_space, key, val, val_size);
}

int hap_keystore_set(const char* name_space, const char* key, const uint8_t* val, const size_t val_len)
{
    char path[64];
    fs_mkdir("hap");
    snprintf(path, 64, "%s/%s", "hap", name_space);
    fs_mkdir(path);
    snprintf(path, 64, "%s/%s/%s", "hap", name_space, key);
    int fd = fs_open(path, "wb");
    if (fd < 0)
        return HAP_FAIL;
    fs_write(val, val_len, fd);
    fs_close(fd);
    return HAP_SUCCESS;
}

int hap_factory_keystore_set(const char* name_space, const char* key, const uint8_t* val, const size_t val_len)
{
    return hap_keystore_set(name_space, key, val, val_len);
}

int hap_keystore_delete(const char* name_space, const char* key)
{
    char path[64];
    snprintf(path, 64, "%s/%s/%s", "hap", name_space, key);
    fs_remove(path);
    return HAP_SUCCESS;
}

int hap_keystore_delete_namespace(const char* name_space)
{
    char path[64];
    snprintf(path, 64, "%s/%s", "hap", name_space);
    fs_remove(path);
    return HAP_SUCCESS;
}

void hap_keystore_erase_all_data(void)
{
    fs_remove("hap");
}

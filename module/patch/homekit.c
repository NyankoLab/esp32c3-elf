#include "esp32c3.h"
#include "module/fs.h"

#define HAP_SUCCESS 0
#define HAP_FAIL    -1
#define TAG         __FILE_NAME__

int hap_keystore_init(void)
{
    return HAP_SUCCESS;
}

int hap_keystore_get(const char* name_space, const char* key, uint8_t* val, size_t* val_size)
{
    sprintf(number, "%s/%s/%s", "hap", name_space, key);
    int fd = fs_open(number, "rb");
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
    fs_mkdir("hap");
    sprintf(number, "%s/%s", "hap", name_space);
    fs_mkdir(number);
    sprintf(number, "%s/%s/%s", "hap", name_space, key);
    int fd = fs_open(number, "wb");
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
    sprintf(number, "%s/%s/%s", "hap", name_space, key);
    fs_remove(number);
    return HAP_SUCCESS;
}

int hap_keystore_delete_namespace(const char* name_space)
{
    sprintf(number, "%s/%s", "hap", name_space);
    fs_remove(number);
    return HAP_SUCCESS;
}

void hap_keystore_erase_all_data(void)
{
    fs_remove("hap");
}

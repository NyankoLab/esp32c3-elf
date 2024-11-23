#include "esp32c3.h"
#include "elf_loader/include/esp_elf.h"

#define TAG __FILE_NAME__

static bool elf_read(void* buffer, size_t size, bool string, const void* data, size_t offset)
{
    FILE* file = (FILE*)data;
    fseek(file, offset, SEEK_SET);
    if (string)
    {
        return fgets(buffer, size, file) ? true : false;
    }
    return fread(buffer, 1, size, file) == size ? true : false;
}

void* dlopen(const char* filename, int flags)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
        return NULL;
    esp_elf_t* elf = malloc(sizeof(esp_elf_t));
    if (elf)
    {
        esp_elf_init(elf);
        int ret = esp_elf_relocate(elf, elf_read, file);
        if (ret != 0)
        {
            printf("Fail to relocate FILE file (%d)\n", ret);
            esp_elf_deinit(elf);
            elf = NULL;
        }
    }
    fclose(file);
    return elf;
}

void* dlsym(void* handle, const char* symbol)
{
    esp_elf_t* elf = (esp_elf_t*)handle;
    if (elf == NULL)
        return NULL;
    return elf->entry;
}

int dlclose(void* handle)
{
    esp_elf_t* elf = (esp_elf_t*)handle;
    if (elf == NULL)
        return -1;
    esp_elf_deinit(elf);
    free(elf);
    return 0;
}

int execv(const char* path, char* const argv[])
{
    int ret = 0;

    void* handle = dlopen(path, 0);
    if (handle) {
        int (*entry)(int argc, char *const argv[]) = dlsym(handle, NULL);
        if (entry) {
            int argc = 0;
            if (argv) {
                for (argc = 0; argv[argc]; ++argc) {
                    
                }
            }
            ESP_LOGI(TAG, "Start to run ELF file");
            ret = entry(argc, argv);
            ESP_LOGI(TAG, "Success to exit from ELF file");
        }
        dlclose(handle);
    }

    return ret;
}

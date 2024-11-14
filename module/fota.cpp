#include "esp32c3.h"
#include <string>
#include <esp_ota_ops.h>
#include <esp_flash.h>
#include <spi_flash_mmap.h>
#include "https.h"
#include "fota.h"

#define TAG __FILE_NAME__

struct fota_context
{
    int offset;
    int size;
    void* temp = nullptr;
    esp_ota_handle_t handle = 0;
    string header;
};

static void fota_recv(void* arg, char* data, int length)
{
    void** other_context = (void**)arg;
    struct fota_context* context = (struct fota_context*)*other_context;

    if (context == NULL)
    {
        *other_context = context = new fota_context;

        context->offset = 0;
        context->size = 0;
        context->temp = realloc(context->temp, 1536);
    }

    if (context->size == 0)
    {
        context->header += string(data, length);
        size_t pos = context->header.find("\r\n\r\n");
        if (pos == string::npos)
            return;
        int skip = length - (context->header.length() - (pos + 4));
        data += skip;
        length -= skip;

        // Content-Length:
        pos = context->header.find("Content-Length: ");
        if (pos != string::npos)
            context->size = strtol(context->header.data() + pos + sizeof("Content-Length: ") - 1, 0, 10);
        if (context->size == 0)
        {
            https_disconnect(arg);
            return;
        }
        context->header = string();
        if (length == 0)
            return;
    }

    if (context->offset == 0 && context->handle == 0)
    {
        esp_ota_begin(esp_ota_get_next_update_partition(nullptr), context->size, &context->handle);
    }
    esp_ota_write_with_offset(context->handle, data, length, context->offset);
    context->offset += length;
    ESP_LOGI(TAG, "%d/%d", context->offset, context->size);
    if (context->offset == context->size)
    {
        esp_ota_end(context->handle);
        context->handle = 0;

        esp_ota_set_boot_partition(esp_ota_get_next_update_partition(nullptr));

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
}

static void fota_disconn(void* arg)
{
    void** other_context = (void**)arg;
    struct fota_context* context = (struct fota_context*)*other_context;

    delete context;
}

void fota(const char* url)
{
    https_connect(url, nullptr, fota_recv, fota_disconn);
}

void fota_callback(void* arg)
{
    https_callback(arg, fota_recv, fota_disconn);
}

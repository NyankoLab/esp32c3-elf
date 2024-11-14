#include "esp32c3.h"
#include <sys/socket.h>
#include <esp_ota_ops.h>
#include <esp_flash.h>
#include <spi_flash_mmap.h>
#include "ota.h"

#define TAG __FILE_NAME__

struct ota_context
{
    int udp_socket;
    int tcp_socket;
    int offset;
    int size;
    void* temp;
    esp_ota_handle_t handle;
};
static struct ota_context* context IRAM_BSS_ATTR;

static void ota_handler(TimerHandle_t timer)
{
    if (context->tcp_socket >= 0)
    {
        char* data = context->temp;
        int length = lwip_recv(context->tcp_socket, data, 1536, 0);
        if (length < 0)
        {
            return;
        }
        if (context->offset == 0 && context->handle == 0)
        {
            esp_ota_begin(esp_ota_get_next_update_partition(NULL), context->size, &context->handle);
        }
        esp_ota_write_with_offset(context->handle, data, length, context->offset);
        context->offset += length;
        ESP_LOGI(TAG, "%d/%d", context->offset, context->size);
        if (length > 0 && length < 1536)
        {
            lwip_send(context->tcp_socket, "OK", 2, 0);
        }
        if (length == 0 || context->offset == context->size)
        {
            if (context->offset == context->size)
            {
                esp_ota_end(context->handle);
                context->handle = 0;

                esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL));

                lwip_send(context->tcp_socket, "OK", 2, 0);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                esp_restart();
            }
            lwip_close(context->tcp_socket);
            context->tcp_socket = -1;

            free(context->temp);
            context->temp = NULL;

            xTimerChangePeriod(timer, 1000 / portTICK_PERIOD_MS, 0);
        }
    }
    else if (context->udp_socket >= 0)
    {
        char data[256];
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        if (lwip_recvfrom(context->udp_socket, data, 256, 0, (struct sockaddr*)&from, &fromlen) > 0)
        {
            char* buffer = data;
            const char* command = strsep(&buffer, " \n");
            const char* remote_port = strsep(&buffer, " \n");
            const char* content_size = strsep(&buffer, " \n");
            const char* file_md5 = strsep(&buffer, " \n");
            if (command && remote_port && content_size && file_md5)
            {
                ESP_LOGI(TAG, "Command: %s", command);
                ESP_LOGI(TAG, "Remote port: %s", remote_port);
                ESP_LOGI(TAG, "Context size: %s", content_size);
                ESP_LOGI(TAG, "File MD5: %s", file_md5);

                context->offset = 0;
                context->size = strtol(content_size, NULL, 10);
                context->temp = realloc(context->temp, 1536);

                context->tcp_socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (context->tcp_socket < 0)
                {
                    lwip_close(FD_SETSIZE - 1);
                    context->tcp_socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                }
                if (context->tcp_socket < 0)
                {
                    ESP_LOGE(TAG, "Create socket failed");
                    return;
                }

                int mode = 1;
                lwip_ioctl(context->tcp_socket, FIONBIO, &mode);

                struct sockaddr_in sockaddr = {};
                sockaddr.sin_len = sizeof(sockaddr);
                sockaddr.sin_family = AF_INET;
                sockaddr.sin_port = htons(strtol(remote_port, NULL, 10));
                sockaddr.sin_addr = ((struct sockaddr_in*)&from)->sin_addr;
                lwip_connect(context->tcp_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

                lwip_sendto(context->udp_socket, "OK", 2, 0, (struct sockaddr*)&from, fromlen);

                xTimerChangePeriod(timer, 10 / portTICK_PERIOD_MS, 0);
            }
        }
    }
}

void ota_init(int port)
{
    if (context == NULL)
    {
        context = calloc(1, sizeof(struct ota_context));
        context->udp_socket = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        context->tcp_socket = -1;

        int mode = 1;
        lwip_ioctl(context->udp_socket, FIONBIO, &mode);

        struct sockaddr_in sockaddr = {};
        sockaddr.sin_len = sizeof(sockaddr);
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(port);
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        lwip_bind(context->udp_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

        TimerHandle_t timer = xTimerCreate("OTA", 1000 / portTICK_PERIOD_MS, pdTRUE, (void*)"OTA", ota_handler);
        xTimerStart(timer, 0);
    }
}

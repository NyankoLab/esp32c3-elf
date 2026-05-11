/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp32c3.h"
#include <stdio.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <esp_app_desc.h>
#include <esp_chip_info.h>
#include <esp_http_server.h>
#include <esp_flash.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/uart_pins.h>
#include "elf_loader/include/esp_elf.h"
#include "module/dlfcn.h"
#include "module/fs.h"
#include "helper.h"

#define TAG __FILE_NAME__

int uart0_tx IRAM_BSS_ATTR = U0TXD_GPIO_NUM;
int uart0_rx IRAM_BSS_ATTR = U0RXD_GPIO_NUM;
int uart1_tx IRAM_BSS_ATTR = U1TXD_GPIO_NUM;
int uart1_rx IRAM_BSS_ATTR = U1RXD_GPIO_NUM;
esp_netif_t* ap_netif IRAM_BSS_ATTR;
esp_netif_t* eth_netif IRAM_BSS_ATTR;
esp_netif_t* sta_netif IRAM_BSS_ATTR;
httpd_handle_t httpd_server IRAM_BSS_ATTR;

// Application version info
const _SECTION_ATTR_IMPL(".rodata_desc", __LINE__) esp_app_desc_t esp_app_desc = {
    .magic_word = ESP_APP_DESC_MAGIC_WORD,
    .secure_version = 0,
    .version = { VersionHelper },
    .project_name = "esp32c3-elf",
    .time = __TIME__,
    .date = __DATE__,
    .idf_ver = "v" __XSTRING(ESP_IDF_VERSION_MAJOR) "." __XSTRING(ESP_IDF_VERSION_MINOR) "." __XSTRING(ESP_IDF_VERSION_PATCH) " "
               "(" "clang version " __XSTRING(__clang_major__) "." __XSTRING(__clang_minor__) "." __XSTRING(__clang_patchlevel__) ")"
};

int mesh_sta_auth_expire_time(void)
{
    return 0;
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    }
#if HAVE_PREPATCH
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        if (fs_stat("preboot") < 0) {
            int file = fs_open("preboot", "w");
            if (file) {
                fs_write("YES", 3, file);
                fs_close(file);
            }

            extern void update();
            update();
        }
    }
#endif
}

void app_main(void)
{
#if 0
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
#endif
    ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes", esp_get_minimum_free_heap_size());

    /* Initialize Component */
    extern void vfs_init(void);
    extern void fs_init(void);
    vfs_init();
    fs_init();

#if HAVE_PREPATCH == 0
    /* Execute ELF */
    if (fs_stat("update") >= 0) {
        fs_remove("update");
        if (execv("update.elf", NULL) < 0) {
            execv("update.old", NULL);
        }
    }
    else {
        execv("main.elf", NULL);
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    /* Fallback */
    extern void wifi_ap(char const* name, char const* pass);
    wifi_ap("ESP32C3", "ESP32C3");
#else
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_config_t config = {};
    config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_connect());
#endif

    /* OTA */
    extern void ota_init(int);
    ota_init(8685);

    for (int i = 200; i >= 0; i--) {
        ESP_LOGI(TAG, "Restarting in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Restarting now.");
    esp_restart();
}

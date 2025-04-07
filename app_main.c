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

    /* Execute ELF */
    execv("main.elf", NULL);

    /* Fallback */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ap_netif = esp_netif_create_default_wifi_ap();

    // WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    // MAC
    uint8_t macaddr[6] = {};
    esp_wifi_get_mac(WIFI_IF_AP, macaddr);

    // Soft AP
    char hostname[24];
    wifi_config_t config = {};
    snprintf((char*)config.ap.ssid, sizeof(config.ap.ssid), "%s-%02X%02X%02X", "ESP32C3", macaddr[3], macaddr[4], macaddr[5]);
    snprintf((char*)config.ap.password, sizeof(config.ap.password), "%s-%02X%02X%02X", "ESP32C3", macaddr[3], macaddr[4], macaddr[5]);
    strcpy(hostname, (char*)config.ap.ssid);
    config.ap.ssid_len = strlen((char*)config.ap.ssid);
    config.ap.channel = 1;
    config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    config.ap.ssid_hidden = 0;
    config.ap.max_connection = 4;
    config.ap.beacon_interval = 100;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_netif_set_hostname(ap_netif, hostname);

    /* OTA */
    extern void ota_init(int);
    ota_init(8685);

    for (int i = 100; i >= 0; i--) {
        ESP_LOGI(TAG, "Restarting in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Restarting now.");
    esp_restart();
}

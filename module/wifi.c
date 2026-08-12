#include "esp32c3.h"
#include <nvs.h>
#include "wifi.h"

#define fgets(o,s,f)            fgets((char*)o, s, f)
#define nvs_get_str(h,n,s,l)    nvs_get_str(h, n, (char*)s, l)
#define nvs_set_str(h,n,s)      nvs_set_str(h, n, (char*)s)
#define snprintf(o,s,f,...)     snprintf((char*)o, s, (char*)f, __VA_ARGS__)
#define strcpy(d,s)             strcpy((char*)d, (char*)s)
#define strlen(s)               strlen((char*)s)

#define TAG __FILE_NAME__

void wifi_ap(char const* name, char const* pass)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ap_netif = esp_netif_create_default_wifi_ap();

    // WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    // MAC
    uint8_t macaddr[6] = {};
    esp_wifi_get_mac(WIFI_IF_STA, macaddr);

    // Soft AP
    wifi_config_t config = {};
    snprintf(config.ap.ssid, sizeof(config.ap.ssid), "%s-%02X%02X%02X", name, macaddr[3], macaddr[4], macaddr[5]);
    snprintf(config.ap.password, sizeof(config.ap.password), "%s-%02X%02X%02X", pass, macaddr[3], macaddr[4], macaddr[5]);
    config.ap.ssid_len = strlen(config.ap.ssid);
    config.ap.channel = 1;
    config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    config.ap.ssid_hidden = 0;
    config.ap.max_connection = 4;
    config.ap.beacon_interval = 100;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Hostname
    char hostname[24];
    strcpy(hostname, (char*)config.ap.ssid);
    esp_netif_set_hostname(ap_netif, hostname);
}

void wifi_sta(char const* name)
{
    ESP_ERROR_CHECK(esp_netif_init());
    sta_netif = esp_netif_create_default_wifi_sta();

    // WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // MAC
    uint8_t macaddr[6] = {};
    esp_wifi_get_mac(WIFI_IF_STA, macaddr);

    // STA
    ESP_ERROR_CHECK(esp_wifi_start());

    // Hostname
    char hostname[24];
    snprintf(hostname, sizeof(hostname), "%s-%02X%02X%02X", name, macaddr[3], macaddr[4], macaddr[5]);
    esp_netif_set_hostname(sta_netif, hostname);
}

void wifi_config(char const* ssid, char const* password, bool connect)
{
    wifi_config_t config = {};
    bool update = false;

    FILE* file = fopen("ssid", "r");
    if (file)
    {
        fgets(config.sta.ssid, sizeof(config.sta.ssid), file);
        fgets(config.sta.password, sizeof(config.sta.password), file);
        fclose(file);
        remove("ssid");
        update = true;
    }
    if (ssid)
    {
        strcpy(config.sta.ssid, ssid);
        update = true;
    }
    if (password)
    {
        strcpy(config.sta.password, password);
        update = true;
    }

    nvs_handle_t handle;
    if (nvs_open("wifi", update ? NVS_READWRITE : NVS_READONLY, &handle) == ESP_OK)
    {
        size_t length = 0;
        if (update)
        {
            nvs_set_str(handle, "ssid", config.sta.ssid);
            nvs_set_str(handle, "password", config.sta.password);
        }
        length = sizeof(config.sta.ssid);
        nvs_get_str(handle, "ssid", config.sta.ssid, &length);
        length = sizeof(config.sta.password);
        nvs_get_str(handle, "password", config.sta.password, &length);
        if (update)
        {
            nvs_commit(handle);
        }
        nvs_close(handle);
    }
    config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    if (connect)
    {
        if (config.sta.ssid[0] == 0)
            config.sta.ssid[0] = '?';
        if (config.sta.password[0] == 0)
            config.sta.password[0] = '?';
//      ESP_LOGI(TAG, "SSID: %s", config.sta.ssid);
//      ESP_LOGI(TAG, "PASSWORD: %s", config.sta.password);
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
}

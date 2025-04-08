#include "esp32c3.h"
#include "wifi.h"

#define snprintf(o,s,f,...) snprintf((char*)o, s, (char*)f, __VA_ARGS__)
#define strcpy(d,s)         strcpy((char*)d, (char*)s)
#define strlen(s)           strlen((char*)s)

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
}

void wifi_sta()
{
    ESP_ERROR_CHECK(esp_netif_init());
    sta_netif = esp_netif_create_default_wifi_sta();

    // WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(esp_wifi_start());
}

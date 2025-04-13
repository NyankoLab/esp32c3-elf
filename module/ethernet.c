/* Ethernet Basic Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp32c3.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "ethernet/ethernet_init.h"
#include "sdkconfig.h"

static int8_t eth_spi_miso_gpio;
static int8_t eth_spi_mosi_gpio;
static int8_t eth_spi_cs0_gpio;
static int8_t eth_spi_sclk_gpio;
static int8_t eth_spi_int0_gpio;
static int8_t eth_spi_phy_rst0_gpio;

#define CONFIG_EXAMPLE_ETH_DEINIT_AFTER_S       -1
#define CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ        16
#define CONFIG_EXAMPLE_ETH_SPI_HOST             1
#define CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO        eth_spi_miso_gpio
#define CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO        eth_spi_mosi_gpio
#define CONFIG_EXAMPLE_ETH_SPI_CS0_GPIO         eth_spi_cs0_gpio
#define CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO        eth_spi_sclk_gpio
#define CONFIG_EXAMPLE_ETH_SPI_INT0_GPIO        eth_spi_int0_gpio
#define CONFIG_EXAMPLE_ETH_SPI_PHY_ADDR0        1
#define CONFIG_EXAMPLE_ETH_SPI_PHY_RST0_GPIO    eth_spi_phy_rst0_gpio
#define CONFIG_EXAMPLE_ETH_SPI_POLLING0_MS      0
#define CONFIG_EXAMPLE_SPI_ETHERNETS_NUM        1
#define CONFIG_EXAMPLE_USE_SPI_ETHERNET         1
#define CONFIG_EXAMPLE_USE_W5500                1

#define TAG __CONCAT(TAG, __LINE__)
#define TAG141 "ethernet"
#define TAG144 "ethernet"
#define TAG158 "ethernet"
#define TAG217 "ethernet"
#define TAG222 "ethernet"
#define TAG254 "ethernet"
#define TAG256 "ethernet"
#define TAG265 "ethernet"
#define TAG273 "ethernet"
#define TAG289 "ethernet"
#define TAG309 "ethernet"
#define TAG316 "ethernet"
#define TAG331 "ethernet"

#include "ethernet/ethernet_init.c"

#undef TAG
#define TAG "ethernet"

static void (*eth_connected_handler)(void);

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        if (eth_connected_handler)
            eth_connected_handler();
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

//  esp_netif_dns_info_t dns = {};
//  esp_netif_get_dns_info(event->esp_netif, ESP_NETIF_DNS_MAIN, &dns);

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
//  ESP_LOGI(TAG, "ETHDNS:" IPSTR, IP2STR(&dns.ip.u_addr.ip4));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
}

void ethernet(char const* name, int miso, int mosi, int scs, int sclk, int interrupt, int reset, void(*connected_handler)(void))
{
    eth_spi_miso_gpio = miso;
    eth_spi_mosi_gpio = mosi;
    eth_spi_cs0_gpio = scs;
    eth_spi_sclk_gpio = sclk;
    eth_spi_int0_gpio = interrupt;
    eth_spi_phy_rst0_gpio = reset;
    eth_connected_handler = connected_handler;

    ESP_LOGI(TAG, "%s : %d", "MISO", miso);
    ESP_LOGI(TAG, "%s : %d", "MOSI", mosi);
    ESP_LOGI(TAG, "%s : %d", "SCS", scs);
    ESP_LOGI(TAG, "%s : %d", "SCLK", sclk);
    ESP_LOGI(TAG, "%s : %d", "INT", interrupt);
    ESP_LOGI(TAG, "%s : %d", "RST", reset);

    // Initialize Ethernet driver
    uint8_t eth_port_cnt = 0;
    esp_eth_handle_t *eth_handles;
    if (example_eth_init(&eth_handles, &eth_port_cnt) != ESP_OK)
        return;

    // Initialize TCP/IP network interface aka the esp-netif (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *eth_netifs[eth_port_cnt];
    esp_eth_netif_glue_handle_t eth_netif_glues[eth_port_cnt];

    // Create instance(s) of esp-netif for Ethernet(s)
    if (CONFIG_EXAMPLE_SPI_ETHERNETS_NUM == 1 || eth_port_cnt == 1) {
        // Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
        // default esp-netif configuration parameters.
        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
        eth_netifs[0] = esp_netif_new(&cfg);
        eth_netif_glues[0] = esp_eth_new_netif_glue(eth_handles[0]);
        // Attach Ethernet driver to TCP/IP stack
        ESP_ERROR_CHECK(esp_netif_attach(eth_netifs[0], eth_netif_glues[0]));
    } else {
        // Use ESP_NETIF_INHERENT_DEFAULT_ETH when multiple Ethernet interfaces are used and so you need to modify
        // esp-netif configuration parameters for each interface (name, priority, etc.).
        esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
        esp_netif_config_t cfg_spi = {
            .base = &esp_netif_config,
            .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
        };
        char if_key_str[10];
        char if_desc_str[10];
        char num_str[3];
        for (int i = 0; i < eth_port_cnt; i++) {
            itoa(i, num_str, 10);
            strcat(strcpy(if_key_str, "ETH_"), num_str);
            strcat(strcpy(if_desc_str, "eth"), num_str);
            esp_netif_config.if_key = if_key_str;
            esp_netif_config.if_desc = if_desc_str;
            esp_netif_config.route_prio -= i*5;
            eth_netifs[i] = esp_netif_new(&cfg_spi);
            eth_netif_glues[i] = esp_eth_new_netif_glue(eth_handles[0]);
            // Attach Ethernet driver to TCP/IP stack
            ESP_ERROR_CHECK(esp_netif_attach(eth_netifs[i], eth_netif_glues[i]));
        }
    }

    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    // Start Ethernet driver state machine
    for (int i = 0; i < eth_port_cnt; i++) {
        ESP_ERROR_CHECK(esp_eth_start(eth_handles[i]));
    }

#if CONFIG_EXAMPLE_ETH_DEINIT_AFTER_S >= 0
    // For demonstration purposes, wait and then deinit Ethernet network
    vTaskDelay(pdMS_TO_TICKS(CONFIG_EXAMPLE_ETH_DEINIT_AFTER_S * 1000));
    ESP_LOGI(TAG, "stop and deinitialize Ethernet network...");
    // Stop Ethernet driver state machine and destroy netif
    for (int i = 0; i < eth_port_cnt; i++) {
        ESP_ERROR_CHECK(esp_eth_stop(eth_handles[i]));
        ESP_ERROR_CHECK(esp_eth_del_netif_glue(eth_netif_glues[i]));
        esp_netif_destroy(eth_netifs[i]);
    }
    esp_netif_deinit();
    ESP_ERROR_CHECK(example_eth_deinit(eth_handles, eth_port_cnt));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, got_ip_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler));
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
#endif // EXAMPLE_ETH_DEINIT_AFTER_S > 0

    eth_netif = eth_netifs[0];

    // MAC
    uint8_t macaddr[6] = {};
    esp_netif_get_mac(eth_netif, macaddr);

    // Hostname
    char hostname[24];
    snprintf(hostname, sizeof(hostname), "%s-%02X%02X%02X", name, macaddr[3], macaddr[4], macaddr[5]);
    esp_netif_set_hostname(eth_netif, hostname);
}

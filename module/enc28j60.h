#pragma once

#ifdef __cplusplus
extern "C" {
#endif

esp_netif_t *enc28j60(int mosi, int miso, int sclk, int cs, int interrupt, void(*connected_handler)(void));

#ifdef __cplusplus
}
#endif

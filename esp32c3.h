#pragma once

#pragma clang diagnostic ignored "-Wstrict-prototypes"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_app_desc.h>
#include <esp_attr.h>
#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

inline int32_t esp_timer_get_time_ms(void)
{
    return (int32_t)(esp_timer_get_time() / 1000);
}
inline int32_t esp_timer_get_time_us(void)
{
    return (int32_t)esp_timer_get_time();
}

extern int uart0_tx;
extern int uart0_rx;
extern int uart1_tx;
extern int uart1_rx;
extern esp_netif_t* ap_netif;
extern esp_netif_t* sta_netif;
extern httpd_handle_t httpd_server;

#ifdef __cplusplus
};
#endif

#ifdef __cplusplus
#include <string>
#endif

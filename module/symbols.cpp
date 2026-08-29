#define memcpy memcpy_used
#define memset memset_used
#define strchr strchr_used
#define strcmp strcmp_used
#define strcspn strcspn_used
#define strerror strerror_used
#define strlen strlen_used
#define strncat strncat_used
#define strrchr strrchr_used
#define _ctype_ _ctype_used
#define __getreent __getreent_unused
#include <algorithm>
#include <array>
#undef memcpy
#undef memset
#undef strchr
#undef strcmp
#undef strcspn
#undef strerror
#undef strlen
#undef strncat
#undef strrchr
#undef _ctype_
#undef __getreent

#define SNTP_MONITOR_SERVER_REACHABILITY 1
#define SNTP_SERVER_DNS 1
#define SNTP_GET_SERVERS_FROM_DHCP 0

#define HAVE_ESPHOME 1
#define HAVE_MATTER 1
#define HAVE_PREPATCH 0

constexpr unsigned int fnv1a(const char* string)
{
#define FNV_32_PRIME 0x01000193
#define FNV_32_INIT  0x811c9dc5
    unsigned int hash = FNV_32_INIT;
    for (char c; (c = (*string)); ++string) {
        hash ^= c;
        hash *= FNV_32_PRIME;
    }
    return hash;
}

struct hashed_elfsym
{
    unsigned int name = 0;
    void* symbol = nullptr;
    constexpr operator unsigned int() const {
        return name;
    }
};

// Base     208
// ESPHome  7
// Matter   28
#define BASE_COUNT      208
#define ESPHOME_COUNT   (HAVE_ESPHOME ? 7 : 0)
#define MATTER_COUNT    (HAVE_MATTER ? 28 : 0)
#define ARRAY_COUNT     BASE_COUNT + ESPHOME_COUNT + MATTER_COUNT

consteval std::array<hashed_elfsym, ARRAY_COUNT> create_customer_table()
{
    std::array<hashed_elfsym, ARRAY_COUNT> table;
    int i = 0;

#define ESP_ELFSYM_EXPORT(_sym) ESP_ELFSYM_EXPORT2(_sym, _sym)
#define ESP_ELFSYM_EXPORT2(_name, _sym) \
    (void)0; \
    auto hash ## _ ## _name = fnv1a(#_name); \
    for (int j = 0; j < i; ++j) { \
        if (table[j].name == hash ## _ ## _name) \
            return {}; \
    } \
    extern int _sym; \
    table[i++] = { hash ## _ ## _name, &_sym }; \
    (void)0
#define ESP_ELFSYM_END (void)0;

    /* string.h */

    ESP_ELFSYM_EXPORT(strerror),
    ESP_ELFSYM_EXPORT(memset),
    ESP_ELFSYM_EXPORT(memcpy),
    ESP_ELFSYM_EXPORT(strlen),
    ESP_ELFSYM_EXPORT(strtod),
    ESP_ELFSYM_EXPORT(strrchr),
    ESP_ELFSYM_EXPORT(strchr),
    ESP_ELFSYM_EXPORT(strcmp),
    ESP_ELFSYM_EXPORT(strtol),
    ESP_ELFSYM_EXPORT(strcspn),
    ESP_ELFSYM_EXPORT(strncat),

    /* stdio.h */

    ESP_ELFSYM_EXPORT(puts),
    ESP_ELFSYM_EXPORT(putchar),
    ESP_ELFSYM_EXPORT(fputc),
    ESP_ELFSYM_EXPORT(fputs),
    ESP_ELFSYM_EXPORT(printf),
    ESP_ELFSYM_EXPORT(vfprintf),
    ESP_ELFSYM_EXPORT(fprintf),
    ESP_ELFSYM_EXPORT(fwrite),

    /* unistd.h */

//  ESP_ELFSYM_EXPORT(usleep),
//  ESP_ELFSYM_EXPORT(sleep),
//  ESP_ELFSYM_EXPORT(exit),
//  ESP_ELFSYM_EXPORT(close),

    /* stdlib.h */

    ESP_ELFSYM_EXPORT(malloc),
    ESP_ELFSYM_EXPORT(calloc),
    ESP_ELFSYM_EXPORT(realloc),
    ESP_ELFSYM_EXPORT(free),

    /* time.h */

    ESP_ELFSYM_EXPORT(clock_gettime),
    ESP_ELFSYM_EXPORT(strftime),

    /* pthread.h */

//  ESP_ELFSYM_EXPORT(pthread_create),
//  ESP_ELFSYM_EXPORT(pthread_attr_init),
//  ESP_ELFSYM_EXPORT(pthread_attr_setstacksize),
//  ESP_ELFSYM_EXPORT(pthread_detach),
//  ESP_ELFSYM_EXPORT(pthread_join),
//  ESP_ELFSYM_EXPORT(pthread_exit),

    /* newlib */

//  ESP_ELFSYM_EXPORT(__errno),
//  ESP_ELFSYM_EXPORT(__getreent),
#ifdef __HAVE_LOCALE_INFO__
//  ESP_ELFSYM_EXPORT(__locale_ctype_ptr),
#else
//  ESP_ELFSYM_EXPORT(_ctype_),
#endif

    /* math */

//  ESP_ELFSYM_EXPORT(__ltdf2),
//  ESP_ELFSYM_EXPORT(__fixunsdfsi),
//  ESP_ELFSYM_EXPORT(__gtdf2),
//  ESP_ELFSYM_EXPORT(__floatunsidf),
//  ESP_ELFSYM_EXPORT(__divdf3),

    /* getopt.h */

//  ESP_ELFSYM_EXPORT(getopt_long),
//  ESP_ELFSYM_EXPORT(optind),
//  ESP_ELFSYM_EXPORT(opterr),
//  ESP_ELFSYM_EXPORT(optarg),
//  ESP_ELFSYM_EXPORT(optopt),

    /* setjmp.h */

//  ESP_ELFSYM_EXPORT(longjmp),
//  ESP_ELFSYM_EXPORT(setjmp),

    /* sys/socket.h */

//  ESP_ELFSYM_EXPORT(lwip_bind),
//  ESP_ELFSYM_EXPORT(lwip_setsockopt),
//  ESP_ELFSYM_EXPORT(lwip_socket),
//  ESP_ELFSYM_EXPORT(lwip_listen),
//  ESP_ELFSYM_EXPORT(lwip_accept),
//  ESP_ELFSYM_EXPORT(lwip_recv),
//  ESP_ELFSYM_EXPORT(lwip_recvfrom),
//  ESP_ELFSYM_EXPORT(lwip_send),
//  ESP_ELFSYM_EXPORT(lwip_sendto),
//  ESP_ELFSYM_EXPORT(lwip_connect),

    /* arpa/inet.h */

//  ESP_ELFSYM_EXPORT(ipaddr_addr),
//  ESP_ELFSYM_EXPORT(lwip_htons),
//  ESP_ELFSYM_EXPORT(lwip_htonl),
//  ESP_ELFSYM_EXPORT(ip4addr_ntoa),

    /* ROM functions */

//  ESP_ELFSYM_EXPORT(ets_printf),

    // c
    ESP_ELFSYM_EXPORT(abort),
    ESP_ELFSYM_EXPORT(dlopen),
    ESP_ELFSYM_EXPORT(dlsym),
    ESP_ELFSYM_EXPORT(dlclose),
    ESP_ELFSYM_EXPORT(execv),
    ESP_ELFSYM_EXPORT(fclose),
    ESP_ELFSYM_EXPORT(fgetc),
    ESP_ELFSYM_EXPORT(fgets),
    ESP_ELFSYM_EXPORT(fopen),
    ESP_ELFSYM_EXPORT(fread),
    ESP_ELFSYM_EXPORT(ftell),
    ESP_ELFSYM_EXPORT(localtime),
    ESP_ELFSYM_EXPORT(localtime_r),
    ESP_ELFSYM_EXPORT(mkdir),
    ESP_ELFSYM_EXPORT(remove),
    ESP_ELFSYM_EXPORT(rename),
    ESP_ELFSYM_EXPORT(sprintf),
    ESP_ELFSYM_EXPORT(snprintf),
    ESP_ELFSYM_EXPORT(setenv),
    ESP_ELFSYM_EXPORT(stat),
    ESP_ELFSYM_EXPORT(time),
    ESP_ELFSYM_EXPORT(tzset),

    // c++
    ESP_ELFSYM_EXPORT(_Znaj),
    ESP_ELFSYM_EXPORT(_Znwj),
    ESP_ELFSYM_EXPORT(_ZdaPv),
    ESP_ELFSYM_EXPORT(_ZdlPv),
    ESP_ELFSYM_EXPORT(_ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE4findEcj),
    ESP_ELFSYM_EXPORT(_ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE4findEPKcj),
    ESP_ELFSYM_EXPORT(_ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE4findERKS5_j),
    ESP_ELFSYM_EXPORT(_ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE5emptyEv),
    ESP_ELFSYM_EXPORT(_ZNKRSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6substrEjj),
    ESP_ELFSYM_EXPORT(_ZNOSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6substrEjj),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE5clearEv),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcj),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6appendEPKc),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6appendEPKcj),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6appendERKS5_),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6assignEPKc),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6resizeEj),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE7replaceEjjPKc),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE9push_backEc),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEaSEOS5_),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEC2ILi0EEEPKc),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED2Ev),

    // c++20
    ESP_ELFSYM_EXPORT2(_ZNKSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6substrEjj, _ZNKRSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6substrEjj);

    // common
    ESP_ELFSYM_EXPORT(init_udp_console),
    ESP_ELFSYM_EXPORT(uart0_tx),
    ESP_ELFSYM_EXPORT(uart0_rx),
    ESP_ELFSYM_EXPORT(uart1_tx),
    ESP_ELFSYM_EXPORT(uart1_rx),
    ESP_ELFSYM_EXPORT(ap_netif),
    ESP_ELFSYM_EXPORT(eth_netif),
    ESP_ELFSYM_EXPORT(sta_netif),
    ESP_ELFSYM_EXPORT(httpd_server),

    // other
    ESP_ELFSYM_EXPORT(esp_app_desc),
    ESP_ELFSYM_EXPORT(esp_clk_apb_freq),
    ESP_ELFSYM_EXPORT(esp_get_idf_version),
    ESP_ELFSYM_EXPORT(esp_restart),
    ESP_ELFSYM_EXPORT(esp_timer_get_time),
//  ESP_ELFSYM_EXPORT(g_wifi_default_wpa_crypto_funcs),
//  ESP_ELFSYM_EXPORT(g_wifi_osi_funcs),
//  ESP_ELFSYM_EXPORT(lwip_ioctl),
//  ESP_ELFSYM_EXPORT(lwip_close),
//  ESP_ELFSYM_EXPORT(periph_module_enable),
//  ESP_ELFSYM_EXPORT(periph_module_reset),
    ESP_ELFSYM_EXPORT(uart_periph_signal),
    ESP_ELFSYM_EXPORT(GPIO_PIN_MUX_REG),
    ESP_ELFSYM_EXPORT(IP_EVENT),
    ESP_ELFSYM_EXPORT(WIFI_EVENT),

    // other - deprecated
    ESP_ELFSYM_EXPORT2(periph_module_enable, fake_periph_module_enable),
    ESP_ELFSYM_EXPORT2(periph_module_reset, fake_periph_module_reset),

    // ethernet
    ESP_ELFSYM_EXPORT(ethernet),

    // httpd
    ESP_ELFSYM_EXPORT(httpd_start),
    ESP_ELFSYM_EXPORT(httpd_register_uri_handler),
    ESP_ELFSYM_EXPORT(httpd_unregister_uri_handler),
    ESP_ELFSYM_EXPORT(httpd_req_get_url_query_len),
    ESP_ELFSYM_EXPORT(httpd_req_get_url_query_str),
    ESP_ELFSYM_EXPORT(httpd_req_url_decode),
    ESP_ELFSYM_EXPORT(httpd_query_key_value),
    ESP_ELFSYM_EXPORT(httpd_query_decode_key_value),
    ESP_ELFSYM_EXPORT(httpd_resp_redirect),
    ESP_ELFSYM_EXPORT(httpd_resp_send),
    ESP_ELFSYM_EXPORT(httpd_resp_send_chunk),
    ESP_ELFSYM_EXPORT(httpd_resp_set_status),
    ESP_ELFSYM_EXPORT(httpd_resp_set_type),
    ESP_ELFSYM_EXPORT(httpd_resp_set_hdr),

    // https
    ESP_ELFSYM_EXPORT(https_connect),
    ESP_ELFSYM_EXPORT(https_disconnect),
    ESP_ELFSYM_EXPORT(https_send),
    ESP_ELFSYM_EXPORT(https_callback),

    // lcd
    ESP_ELFSYM_EXPORT(lcd_init),
    ESP_ELFSYM_EXPORT(lcd_text),

    // mqtt
    ESP_ELFSYM_EXPORT(mqtt_prefix),
    ESP_ELFSYM_EXPORT(mqtt_publish),
    ESP_ELFSYM_EXPORT(mqtt_receive),
    ESP_ELFSYM_EXPORT(mqtt_setup),
    ESP_ELFSYM_EXPORT(mqtt_connected),
    ESP_ELFSYM_EXPORT(mqtt_connected_internal),
    ESP_ELFSYM_EXPORT(mqtt_ready_internal),

    // nvs
    ESP_ELFSYM_EXPORT(nvs_open),
    ESP_ELFSYM_EXPORT(nvs_open_from_partition),
    ESP_ELFSYM_EXPORT(nvs_set_i8),
    ESP_ELFSYM_EXPORT(nvs_set_u8),
    ESP_ELFSYM_EXPORT(nvs_set_i16),
    ESP_ELFSYM_EXPORT(nvs_set_u16),
    ESP_ELFSYM_EXPORT(nvs_set_i32),
    ESP_ELFSYM_EXPORT(nvs_set_u32),
    ESP_ELFSYM_EXPORT(nvs_set_i64),
    ESP_ELFSYM_EXPORT(nvs_set_u64),
    ESP_ELFSYM_EXPORT(nvs_set_str),
    ESP_ELFSYM_EXPORT(nvs_set_blob),
    ESP_ELFSYM_EXPORT(nvs_get_i8),
    ESP_ELFSYM_EXPORT(nvs_get_u8),
    ESP_ELFSYM_EXPORT(nvs_get_i16),
    ESP_ELFSYM_EXPORT(nvs_get_u16),
    ESP_ELFSYM_EXPORT(nvs_get_i32),
    ESP_ELFSYM_EXPORT(nvs_get_u32),
    ESP_ELFSYM_EXPORT(nvs_get_i64),
    ESP_ELFSYM_EXPORT(nvs_get_u64),
    ESP_ELFSYM_EXPORT(nvs_get_str),
    ESP_ELFSYM_EXPORT(nvs_get_blob),
    ESP_ELFSYM_EXPORT(nvs_erase_key),
    ESP_ELFSYM_EXPORT(nvs_erase_all),
    ESP_ELFSYM_EXPORT(nvs_commit),
    ESP_ELFSYM_EXPORT(nvs_close),

    // ota
    ESP_ELFSYM_EXPORT(ota_init),

    // sntp
    ESP_ELFSYM_EXPORT(sntp_setoperatingmode),
    ESP_ELFSYM_EXPORT(sntp_getoperatingmode),
    ESP_ELFSYM_EXPORT(sntp_init),
    ESP_ELFSYM_EXPORT(sntp_stop),
    ESP_ELFSYM_EXPORT(sntp_enabled),
    ESP_ELFSYM_EXPORT(sntp_setserver),
    ESP_ELFSYM_EXPORT(sntp_getserver),
#if SNTP_MONITOR_SERVER_REACHABILITY
    ESP_ELFSYM_EXPORT(sntp_getreachability),
#endif
#if SNTP_SERVER_DNS
    ESP_ELFSYM_EXPORT(sntp_setservername),
    ESP_ELFSYM_EXPORT(sntp_getservername),
#endif
#if SNTP_GET_SERVERS_FROM_DHCP || SNTP_GET_SERVERS_FROM_DHCPV6
    ESP_ELFSYM_EXPORT(sntp_servermode_dhcp),
#endif

    // i2c
//  ESP_ELFSYM_EXPORT(i2c_new_master_bus),

    // spi_slave
    ESP_ELFSYM_EXPORT(spi_slave_initialize),
//  ESP_ELFSYM_EXPORT(spi_slave_free),
    ESP_ELFSYM_EXPORT(spi_slave_queue_trans),
    ESP_ELFSYM_EXPORT(spi_slave_get_trans_result),
    ESP_ELFSYM_EXPORT(spi_slave_transmit),

    // temperature
    ESP_ELFSYM_EXPORT(temperature_init),
    ESP_ELFSYM_EXPORT(temperature),

    // wifi
    ESP_ELFSYM_EXPORT(wifi_ap),
    ESP_ELFSYM_EXPORT(wifi_sta),
    ESP_ELFSYM_EXPORT(wifi_config),

    // esp_adc
//  ESP_ELFSYM_EXPORT(esp_adc_cal_characterize),
//  ESP_ELFSYM_EXPORT(esp_adc_cal_raw_to_voltage),
//  ESP_ELFSYM_EXPORT(esp_adc_cal_get_voltage),
    ESP_ELFSYM_EXPORT(adc_oneshot_config_channel),
    ESP_ELFSYM_EXPORT(adc_oneshot_new_unit),
    ESP_ELFSYM_EXPORT(adc_oneshot_read),
    ESP_ELFSYM_EXPORT(adc_cali_create_scheme_curve_fitting),
    ESP_ELFSYM_EXPORT(adc_cali_raw_to_voltage),

    // esp_event
//  ESP_ELFSYM_EXPORT(esp_event_loop_create),
//  ESP_ELFSYM_EXPORT(esp_event_loop_delete),
    ESP_ELFSYM_EXPORT(esp_event_loop_create_default),
//  ESP_ELFSYM_EXPORT(esp_event_loop_delete_default),
//  ESP_ELFSYM_EXPORT(esp_event_loop_run),
//  ESP_ELFSYM_EXPORT(esp_event_handler_register),
//  ESP_ELFSYM_EXPORT(esp_event_handler_register_with),
//  ESP_ELFSYM_EXPORT(esp_event_handler_instance_register_with),
    ESP_ELFSYM_EXPORT(esp_event_handler_instance_register),
//  ESP_ELFSYM_EXPORT(esp_event_handler_unregister),
//  ESP_ELFSYM_EXPORT(esp_event_handler_unregister_with),
//  ESP_ELFSYM_EXPORT(esp_event_handler_instance_unregister_with),
//  ESP_ELFSYM_EXPORT(esp_event_handler_instance_unregister),
//  ESP_ELFSYM_EXPORT(esp_event_post),
//  ESP_ELFSYM_EXPORT(esp_event_post_to),
#if CONFIG_ESP_EVENT_POST_FROM_ISR
//  ESP_ELFSYM_EXPORT(esp_event_isr_post),
//  ESP_ELFSYM_EXPORT(esp_event_isr_post_to),
#endif
//  ESP_ELFSYM_EXPORT(esp_event_dump),

    // esp_lcd
//  ESP_ELFSYM_EXPORT(esp_lcd_new_panel_io_i2c_v1),
//  ESP_ELFSYM_EXPORT(esp_lcd_new_panel_io_i2c_v2),
//  ESP_ELFSYM_EXPORT(esp_lcd_new_panel_ssd1306),
//  ESP_ELFSYM_EXPORT(esp_lcd_panel_init),
//  ESP_ELFSYM_EXPORT(esp_lcd_panel_disp_on_off),
//  ESP_ELFSYM_EXPORT(esp_lcd_panel_draw_bitmap),
//  ESP_ELFSYM_EXPORT(esp_lcd_panel_reset),

    // esp_log
    ESP_ELFSYM_EXPORT(esp_log),
//  ESP_ELFSYM_EXPORT(esp_log_level_set),
//  ESP_ELFSYM_EXPORT(esp_log_level_get),
//  ESP_ELFSYM_EXPORT(esp_log_set_vprintf),
    ESP_ELFSYM_EXPORT(esp_log_timestamp),
//  ESP_ELFSYM_EXPORT(esp_log_system_timestamp),
//  ESP_ELFSYM_EXPORT(esp_log_early_timestamp),
    ESP_ELFSYM_EXPORT(esp_log_write),
//  ESP_ELFSYM_EXPORT(esp_log_writev),

    // esp_netif
    ESP_ELFSYM_EXPORT(esp_netif_init),
//  ESP_ELFSYM_EXPORT(esp_netif_deinit),
//  ESP_ELFSYM_EXPORT(esp_netif_new),
//  ESP_ELFSYM_EXPORT(esp_netif_destroy),
//  ESP_ELFSYM_EXPORT(esp_netif_set_driver_config),
//  ESP_ELFSYM_EXPORT(esp_netif_attach),
//  ESP_ELFSYM_EXPORT(esp_netif_receive),
//  ESP_ELFSYM_EXPORT(esp_netif_action_start),
//  ESP_ELFSYM_EXPORT(esp_netif_action_stop),
//  ESP_ELFSYM_EXPORT(esp_netif_action_connected),
//  ESP_ELFSYM_EXPORT(esp_netif_action_disconnected),
//  ESP_ELFSYM_EXPORT(esp_netif_action_got_ip),
//  ESP_ELFSYM_EXPORT(esp_netif_action_join_ip6_multicast_group),
//  ESP_ELFSYM_EXPORT(esp_netif_action_leave_ip6_multicast_group),
//  ESP_ELFSYM_EXPORT(esp_netif_action_add_ip6_address),
//  ESP_ELFSYM_EXPORT(esp_netif_action_remove_ip6_address),
//  ESP_ELFSYM_EXPORT(esp_netif_set_default_netif),
//  ESP_ELFSYM_EXPORT(esp_netif_get_default_netif),
#if CONFIG_ESP_NETIF_BRIDGE_EN
//  ESP_ELFSYM_EXPORT(esp_netif_bridge_add_port),
//  ESP_ELFSYM_EXPORT(esp_netif_bridge_fdb_add),
//  ESP_ELFSYM_EXPORT(esp_netif_bridge_fdb_remove),
#endif
//  ESP_ELFSYM_EXPORT(esp_netif_join_ip6_multicast_group),
//  ESP_ELFSYM_EXPORT(esp_netif_leave_ip6_multicast_group),
//  ESP_ELFSYM_EXPORT(esp_netif_set_mac),
    ESP_ELFSYM_EXPORT(esp_netif_get_mac),
    ESP_ELFSYM_EXPORT(esp_netif_set_hostname),
    ESP_ELFSYM_EXPORT(esp_netif_get_hostname),
//  ESP_ELFSYM_EXPORT(esp_netif_is_netif_up),
    ESP_ELFSYM_EXPORT(esp_netif_get_ip_info),
//  ESP_ELFSYM_EXPORT(esp_netif_get_old_ip_info),
//  ESP_ELFSYM_EXPORT(esp_netif_set_ip_info),
//  ESP_ELFSYM_EXPORT(esp_netif_set_old_ip_info),
//  ESP_ELFSYM_EXPORT(esp_netif_get_netif_impl_index),
//  ESP_ELFSYM_EXPORT(esp_netif_get_netif_impl_name),
//  ESP_ELFSYM_EXPORT(esp_netif_napt_enable),
//  ESP_ELFSYM_EXPORT(esp_netif_napt_disable),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcps_option),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcpc_option),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcpc_start),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcpc_stop),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcpc_get_status),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcps_get_status),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcps_start),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcps_stop),
//  ESP_ELFSYM_EXPORT(esp_netif_dhcps_get_clients_by_mac),
//  ESP_ELFSYM_EXPORT(esp_netif_set_dns_info),
//  ESP_ELFSYM_EXPORT(esp_netif_get_dns_info),
#if CONFIG_LWIP_IPV6
//  ESP_ELFSYM_EXPORT(esp_netif_create_ip6_linklocal),
//  ESP_ELFSYM_EXPORT(esp_netif_get_ip6_linklocal),
//  ESP_ELFSYM_EXPORT(esp_netif_get_ip6_global),
//  ESP_ELFSYM_EXPORT(esp_netif_get_all_ip6),
//  ESP_ELFSYM_EXPORT(esp_netif_add_ip6_address),
//  ESP_ELFSYM_EXPORT(esp_netif_remove_ip6_address),
#endif
//  ESP_ELFSYM_EXPORT(esp_netif_set_ip4_addr),
    ESP_ELFSYM_EXPORT(esp_ip4addr_ntoa),
//  ESP_ELFSYM_EXPORT(esp_ip4addr_aton),
//  ESP_ELFSYM_EXPORT(esp_netif_str_to_ip4),
//  ESP_ELFSYM_EXPORT(esp_netif_str_to_ip6),
//  ESP_ELFSYM_EXPORT(esp_netif_get_io_driver),
//  ESP_ELFSYM_EXPORT(esp_netif_get_handle_from_ifkey),
//  ESP_ELFSYM_EXPORT(esp_netif_get_flags),
//  ESP_ELFSYM_EXPORT(esp_netif_get_ifkey),
//  ESP_ELFSYM_EXPORT(esp_netif_get_desc),
//  ESP_ELFSYM_EXPORT(esp_netif_get_route_prio),
//  ESP_ELFSYM_EXPORT(esp_netif_get_event_id),
//  ESP_ELFSYM_EXPORT(esp_netif_next_unsafe),
//  ESP_ELFSYM_EXPORT(esp_netif_find_if),
//  ESP_ELFSYM_EXPORT(esp_netif_get_nr_of_ifs),
//  ESP_ELFSYM_EXPORT(esp_netif_netstack_buf_ref),
//  ESP_ELFSYM_EXPORT(esp_netif_netstack_buf_free),
//  ESP_ELFSYM_EXPORT(esp_netif_tcpip_exec),

    // esp_ota
    ESP_ELFSYM_EXPORT(esp_ota_begin),
//  ESP_ELFSYM_EXPORT(esp_ota_write),
    ESP_ELFSYM_EXPORT(esp_ota_write_with_offset),
    ESP_ELFSYM_EXPORT(esp_ota_end),
//  ESP_ELFSYM_EXPORT(esp_ota_abort),
    ESP_ELFSYM_EXPORT(esp_ota_set_boot_partition),
//  ESP_ELFSYM_EXPORT(esp_ota_get_boot_partition),
//  ESP_ELFSYM_EXPORT(esp_ota_get_running_partition),
    ESP_ELFSYM_EXPORT(esp_ota_get_next_update_partition),
//  ESP_ELFSYM_EXPORT(esp_ota_get_partition_description),
//  ESP_ELFSYM_EXPORT(esp_ota_get_bootloader_description),
//  ESP_ELFSYM_EXPORT(esp_ota_get_app_partition_count),
//  ESP_ELFSYM_EXPORT(esp_ota_mark_app_valid_cancel_rollback),
//  ESP_ELFSYM_EXPORT(esp_ota_mark_app_invalid_rollback_and_reboot),
//  ESP_ELFSYM_EXPORT(esp_ota_get_last_invalid_partition),
//  ESP_ELFSYM_EXPORT(esp_ota_get_state_partition),
//  ESP_ELFSYM_EXPORT(esp_ota_erase_last_boot_app_partition),
//  ESP_ELFSYM_EXPORT(esp_ota_check_rollback_is_possible),

    // esp_wifi
    ESP_ELFSYM_EXPORT(esp_wifi_init),
//  ESP_ELFSYM_EXPORT(esp_wifi_deinit),
    ESP_ELFSYM_EXPORT(esp_wifi_set_mode),
    ESP_ELFSYM_EXPORT(esp_wifi_get_mode),
    ESP_ELFSYM_EXPORT(esp_wifi_start),
//  ESP_ELFSYM_EXPORT(esp_wifi_stop),
//  ESP_ELFSYM_EXPORT(esp_wifi_restore),
    ESP_ELFSYM_EXPORT(esp_wifi_connect),
    ESP_ELFSYM_EXPORT(esp_wifi_disconnect),
//  ESP_ELFSYM_EXPORT(esp_wifi_clear_fast_connect),
//  ESP_ELFSYM_EXPORT(esp_wifi_deauth_sta),
//  ESP_ELFSYM_EXPORT(esp_wifi_scan_start),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_scan_parameters),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_scan_parameters),
//  ESP_ELFSYM_EXPORT(esp_wifi_scan_stop),
//  ESP_ELFSYM_EXPORT(esp_wifi_scan_get_ap_num),
//  ESP_ELFSYM_EXPORT(esp_wifi_scan_get_ap_records),
//  ESP_ELFSYM_EXPORT(esp_wifi_scan_get_ap_record),
//  ESP_ELFSYM_EXPORT(esp_wifi_clear_ap_list),
    ESP_ELFSYM_EXPORT(esp_wifi_sta_get_ap_info),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_ps),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_ps),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_protocol),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_protocol),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_bandwidth),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_bandwidth),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_channel),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_channel),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_country),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_country),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_mac),
    ESP_ELFSYM_EXPORT(esp_wifi_get_mac),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous_rx_cb),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_promiscuous),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous_filter),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_promiscuous_filter),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous_ctrl_filter),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_promiscuous_ctrl_filter),
    ESP_ELFSYM_EXPORT(esp_wifi_set_config),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_config),
    ESP_ELFSYM_EXPORT(esp_wifi_ap_get_sta_list),
//  ESP_ELFSYM_EXPORT(esp_wifi_ap_get_sta_aid),
    ESP_ELFSYM_EXPORT(esp_wifi_set_storage),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_vendor_ie),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_vendor_ie_cb),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_max_tx_power),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_max_tx_power),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_event_mask),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_event_mask),
//  ESP_ELFSYM_EXPORT(esp_wifi_80211_tx),
#if 0
    ESP_ELFSYM_EXPORT(esp_wifi_set_csi_rx_cb),
    ESP_ELFSYM_EXPORT(esp_wifi_set_csi_config),
    ESP_ELFSYM_EXPORT(esp_wifi_get_csi_config),
    ESP_ELFSYM_EXPORT(esp_wifi_set_csi),
#endif
//  ESP_ELFSYM_EXPORT(esp_wifi_get_tsf_time),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_inactive_time),
//  ESP_ELFSYM_EXPORT(esp_wifi_statis_dump),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_rssi_threshold),
//  ESP_ELFSYM_EXPORT(esp_wifi_ftm_initiate_session),
//  ESP_ELFSYM_EXPORT(esp_wifi_ftm_end_session),
//  ESP_ELFSYM_EXPORT(esp_wifi_ftm_resp_set_offset),
//  ESP_ELFSYM_EXPORT(esp_wifi_ftm_get_report),
//  ESP_ELFSYM_EXPORT(esp_wifi_config_11b_rate),
//  ESP_ELFSYM_EXPORT(esp_wifi_connectionless_module_set_wake_interval),
//  ESP_ELFSYM_EXPORT(esp_wifi_force_wakeup_acquire),
//  ESP_ELFSYM_EXPORT(esp_wifi_force_wakeup_release),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_country_code),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_country_code),
//  ESP_ELFSYM_EXPORT(esp_wifi_config_80211_tx_rate),
//  ESP_ELFSYM_EXPORT(esp_wifi_disable_pmf_config),
//  ESP_ELFSYM_EXPORT(esp_wifi_sta_get_aid),
//  ESP_ELFSYM_EXPORT(esp_wifi_sta_get_negotiated_phymode),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_dynamic_cs),
//  ESP_ELFSYM_EXPORT(esp_wifi_sta_get_rssi),

    // esp_wifi_default
//  ESP_ELFSYM_EXPORT(esp_netif_attach_wifi_station),
//  ESP_ELFSYM_EXPORT(esp_netif_attach_wifi_ap),
    ESP_ELFSYM_EXPORT(esp_wifi_set_default_wifi_sta_handlers),
    ESP_ELFSYM_EXPORT(esp_wifi_set_default_wifi_ap_handlers),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_default_wifi_nan_handlers),
    ESP_ELFSYM_EXPORT(esp_wifi_clear_default_wifi_driver_and_handlers),
    ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_ap),
    ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_sta),
//  ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_nan),
//  ESP_ELFSYM_EXPORT(esp_netif_destroy_default_wifi),
//  ESP_ELFSYM_EXPORT(esp_netif_create_wifi),
//  ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_mesh_netifs),

    // freertos
    ESP_ELFSYM_EXPORT(vTaskDelay),
    ESP_ELFSYM_EXPORT(vTaskDelete),
    ESP_ELFSYM_EXPORT(xTaskCreatePinnedToCore),
    ESP_ELFSYM_EXPORT(xTaskGetTickCount),
    ESP_ELFSYM_EXPORT(xTimerCreate),
    ESP_ELFSYM_EXPORT(xTimerGenericCommand),

#if HAVE_ESPHOME
//  ESP_ELFSYM_EXPORT(_ZN7ESPHome3API4SendEiiz),
    ESP_ELFSYM_EXPORT(_ZN7ESPHome3API4SendEiiPv),
    ESP_ELFSYM_EXPORT(_ZN7ESPHome6Server9BroadcastEiPv),
    ESP_ELFSYM_EXPORT(_ZN7ESPHome6Server5StartEPFviiPKvE),
    ESP_ELFSYM_EXPORT(_ZN7ESPHome5Setup4NameE),
    ESP_ELFSYM_EXPORT(_ZN7ESPHome5Setup5ModelE),
    ESP_ELFSYM_EXPORT(_ZN7ESPHome5Setup12ManufacturerE),
    ESP_ELFSYM_EXPORT(_ZN7ESPHome5Setup12FriendlyNameE),
#endif

#if HAVE_MATTER
    // chip
    ESP_ELFSYM_EXPORT(_ZN4chip26CommissioningWindowManager28OpenBasicCommissioningWindowENSt3__16chrono8durationIjNS1_5ratioILx1ELx1EEEEENS_32CommissioningWindowAdvertisementE),
    ESP_ELFSYM_EXPORT(_ZNK4chip26CommissioningWindowManager25IsCommissioningWindowOpenEv),
    ESP_ELFSYM_EXPORT(_ZN4chip11DeviceLayer29SetDeviceInstanceInfoProviderEPNS0_26DeviceInstanceInfoProviderE),
    ESP_ELFSYM_EXPORT(_ZN4chip6Server7sServerE),

    // esp-matter
    ESP_ELFSYM_EXPORT(_ZN10esp_matter20default_app_event_cbEPKN4chip11DeviceLayer15ChipDeviceEventEi),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter29default_app_identification_cbENS_14identification13callback_typeEthhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter13factory_resetEv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter12get_passcodeEv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter17get_discriminatorEv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter16get_fabric_countEv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter16get_pairing_codeERNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter10get_qrcodeERNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter10is_startedEv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter4node6createEPNS0_6configEPFiNS_9attribute13callback_typeEtjjP19esp_matter_attr_valPvEPFiNS_14identification13callback_typeEthhS7_ES7_),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter11set_productEPKc),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter10set_vendorEPKc),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter11set_versionEPKc),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter5startEPFvPKN4chip11DeviceLayer15ChipDeviceEventEiEi),

    // attribute
    ESP_ELFSYM_EXPORT(_ZN10esp_matter9attribute10add_boundsEPj19esp_matter_attr_valS2_),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter9attribute6createEPjjt19esp_matter_attr_valt),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter9attribute7get_valEtjjP19esp_matter_attr_val),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter9attribute6reportEtjjP19esp_matter_attr_val),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter9attribute6updateEtjjP19esp_matter_attr_val),

    // cluster
    ESP_ELFSYM_EXPORT(_ZN10esp_matter7cluster6createEiPjPvh);
    ESP_ELFSYM_EXPORT(_ZN10esp_matter7cluster3getEPjj),

    // endpoint
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint3addEiPjPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint6createEiPjPvhS2_),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint6get_idEPj),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint13get_priv_dataEt),

    // notify
    ESP_ELFSYM_EXPORT(_Z38MatterReportingAttributeChangeCallbacktjj),
#if 0
    // endpoint - 1.0
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint9door_lock3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint9door_lock6createEPjPNS1_6configEhPv),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20door_lock_controller3addEPjPNS1_6configE),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20door_lock_controller6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15window_covering3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15window_covering6createEPjPNS1_6configEhPv),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint26window_covering_controller3addEPjPNS1_6configE),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint26window_covering_controller6createEPjPNS1_6configEhPv),

    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint18temperature_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint18temperature_sensor6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15humidity_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15humidity_sensor6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint16occupancy_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint16occupancy_sensor6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12light_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12light_sensor6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14contact_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14contact_sensor6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15pressure_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15pressure_sensor6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11flow_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11flow_sensor6createEPjPNS1_6configEhPv),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint13on_off_sensor3addEPjPNS1_6configE),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint13on_off_sensor6createEPjPNS1_6configEhPv),

    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint10thermostat3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint10thermostat6createEPjPNS1_6configEhPv),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20heating_cooling_unit3addEPjPNS1_6configE),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20heating_cooling_unit6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint4pump3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint4pump6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15pump_controller3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15pump_controller6createEPjPNS1_6configEhPv),

    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19on_off_plug_in_unit3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19on_off_plug_in_unit6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint21dimmable_plug_in_unit3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint21dimmable_plug_in_unit6createEPjPNS1_6configEhPv),

    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20extended_color_light3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20extended_color_light6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint23color_temperature_light3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint23color_temperature_light6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12on_off_light3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12on_off_light6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14dimmable_light3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14dimmable_light6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19on_off_light_switch3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19on_off_light_switch6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint13dimmer_switch3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint13dimmer_switch6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19color_dimmer_switch3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19color_dimmer_switch6createEPjPNS1_6configEhPv),

    // endpoint - 1.2
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint18air_quality_sensor3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint18air_quality_sensor6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14smoke_co_alarm3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14smoke_co_alarm6createEPjPNS1_6configEhPv),

    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint3fan3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint3fan6createEPjPNS1_6configEhPv),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12air_purifier3addEPjPNS1_6configE),
    ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12air_purifier6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20room_air_conditioner3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint20room_air_conditioner6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint22robotic_vacuum_cleaner3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint22robotic_vacuum_cleaner6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint30temperature_controlled_cabinet3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint30temperature_controlled_cabinet6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12refrigerator3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12refrigerator6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14laundry_washer3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14laundry_washer6createEPjPNS1_6configEhPv),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint10dishwasher3addEPjPNS1_6configE),
//??ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint10dishwasher6createEPjPNS1_6configEhPv),

    // endpoint - 1.3
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11water_valve3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11water_valve6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19water_leak_detector3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint19water_leak_detector6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint21water_freeze_detector3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint21water_freeze_detector6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12cook_surface3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12cook_surface6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint7cooktop3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint7cooktop6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14microwave_oven3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14microwave_oven6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14extractor_hood3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint14extractor_hood6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint4oven3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint4oven6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint13laundry_dryer3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint13laundry_dryer6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11rain_sensor3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11rain_sensor6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11energy_evse3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11energy_evse6createEPjPNS1_6configEhPv),

    // endpoint - 1.4
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint17electrical_sensor3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint17electrical_sensor6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint24device_energy_management3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint24device_energy_management6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint21thermostat_controller3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint21thermostat_controller6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint22mounted_on_off_control3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint22mounted_on_off_control6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11solar_power3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint11solar_power6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15battery_storage3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint15battery_storage6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint9heat_pump3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint9heat_pump6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12water_heater3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint12water_heater6createEPjPNS1_6configEhPv),

//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint29mounted_dimmable_load_control6createEPjPNS0_14dimmable_light6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint29mounted_dimmable_load_control3addEPjPNS0_14dimmable_light6configE),

    // endpoint - 1.5
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint6camera3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint6camera6createEPjPNS1_6configEhPv),

    // endpoint - ???
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint5chime3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint5chime6createEPjPNS1_6configEhPv),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint7closure3addEPjPNS1_6configE),
//  ESP_ELFSYM_EXPORT(_ZN10esp_matter8endpoint7closure6createEPjPNS1_6configEhPv),
#endif
#endif

    // end
    ESP_ELFSYM_END

    std::sort(table.begin(), table.end());
    return table;
};

static constexpr const auto g_customer_elfsyms = create_customer_table();
static_assert(g_customer_elfsyms[0].name != 0);

extern "C" void* __wrap_elf_find_sym(const char* sym_name)
{
    auto hash = fnv1a(sym_name);
    auto it = std::lower_bound(g_customer_elfsyms.begin(), g_customer_elfsyms.end(), hash);
    if (it != g_customer_elfsyms.end() && (*it).name == hash)
        return (*it).symbol;
    return nullptr;
}

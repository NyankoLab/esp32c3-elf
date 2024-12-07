#include "esp32c3.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <esp_ota_ops.h>
#include <esp_private/esp_clk.h>
#include <esp_private/periph_ctrl.h>
#include <soc/gpio_periph.h>
#include <soc/uart_periph.h>
#include <lwip/apps/sntp.h>
#include <hap.h>
#include <hap_apple_chars.h>
#include <hap_apple_servs.h>
#include <esp_hap_controllers.h>
#include <esp_hap_database.h>
#include "dlfcn.h"
#include "httpd.h"
#include "https.h"
#include "mqtt.h"
#include "ota.h"
#include "temperature.h"
#include "elf_loader/include/private/elf_symbol.h"

void* _Znaj(unsigned int s) { return malloc(s); }
void* _Znwj(unsigned int s) { return malloc(s); }
void _ZdaPv(void* p) { free(p); }
void _ZdlPv(void* p) { free(p); }

void init_udp_console(const char* ip);
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t, const char* const, const configSTACK_DEPTH_TYPE, void* const, UBaseType_t, TaskHandle_t* const, const BaseType_t);
extern esp_app_desc_t esp_app_desc;

const struct esp_elfsym g_customer_elfsyms[] = {

    // c
    ESP_ELFSYM_EXPORT(abort),
    ESP_ELFSYM_EXPORT(dlopen),
    ESP_ELFSYM_EXPORT(dlsym),
    ESP_ELFSYM_EXPORT(dlclose),
    ESP_ELFSYM_EXPORT(ftell),
    ESP_ELFSYM_EXPORT(fopen),
    ESP_ELFSYM_EXPORT(fclose),
    ESP_ELFSYM_EXPORT(fgetc),
    ESP_ELFSYM_EXPORT(fgets),
    ESP_ELFSYM_EXPORT(fread),
    ESP_ELFSYM_EXPORT(mkdir),
    ESP_ELFSYM_EXPORT(remove),
    ESP_ELFSYM_EXPORT(sprintf),
    ESP_ELFSYM_EXPORT(snprintf),
    ESP_ELFSYM_EXPORT(stat),
    ESP_ELFSYM_EXPORT(time),
    ESP_ELFSYM_EXPORT(localtime),
    ESP_ELFSYM_EXPORT(localtime_r),
    ESP_ELFSYM_EXPORT(setenv),
    ESP_ELFSYM_EXPORT(tzset),

    // c++
    ESP_ELFSYM_EXPORT(_Znaj),
    ESP_ELFSYM_EXPORT(_Znwj),
    ESP_ELFSYM_EXPORT(_ZdaPv),
    ESP_ELFSYM_EXPORT(_ZdlPv),

    // common
    ESP_ELFSYM_EXPORT(init_udp_console),
    ESP_ELFSYM_EXPORT(execv),
    ESP_ELFSYM_EXPORT(uart0_tx),
    ESP_ELFSYM_EXPORT(uart0_rx),
    ESP_ELFSYM_EXPORT(uart1_tx),
    ESP_ELFSYM_EXPORT(uart1_rx),
    ESP_ELFSYM_EXPORT(ap_netif),
    ESP_ELFSYM_EXPORT(sta_netif),
    ESP_ELFSYM_EXPORT(httpd_server),

    // other
    ESP_ELFSYM_EXPORT(esp_app_desc),
    ESP_ELFSYM_EXPORT(esp_clk_apb_freq),
    ESP_ELFSYM_EXPORT(esp_get_idf_version),
    ESP_ELFSYM_EXPORT(esp_restart),
    ESP_ELFSYM_EXPORT(esp_timer_get_time),
    ESP_ELFSYM_EXPORT(g_wifi_default_wpa_crypto_funcs),
    ESP_ELFSYM_EXPORT(g_wifi_osi_funcs),
    ESP_ELFSYM_EXPORT(lwip_ioctl),
    ESP_ELFSYM_EXPORT(lwip_close),
    ESP_ELFSYM_EXPORT(periph_module_enable),
    ESP_ELFSYM_EXPORT(periph_module_reset),
    ESP_ELFSYM_EXPORT(uart_periph_signal),
    ESP_ELFSYM_EXPORT(GPIO_PIN_MUX_REG),
    ESP_ELFSYM_EXPORT(IP_EVENT),
    ESP_ELFSYM_EXPORT(WIFI_EVENT),

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

    // mqtt
    ESP_ELFSYM_EXPORT(mqtt_prefix),
    ESP_ELFSYM_EXPORT(mqtt_publish),
    ESP_ELFSYM_EXPORT(mqtt_receive),
    ESP_ELFSYM_EXPORT(mqtt_setup),
    ESP_ELFSYM_EXPORT(mqtt_connected),
    ESP_ELFSYM_EXPORT(mqtt_connected_internal),
    ESP_ELFSYM_EXPORT(mqtt_ready_internal),

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

    // temperature
    ESP_ELFSYM_EXPORT(temperature_init),
    ESP_ELFSYM_EXPORT(temperature),

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

    // esp_log
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
//  ESP_ELFSYM_EXPORT(esp_netif_get_mac),
    ESP_ELFSYM_EXPORT(esp_netif_set_hostname),
//  ESP_ELFSYM_EXPORT(esp_netif_get_hostname),
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

    // hap
//  ESP_ELFSYM_EXPORT(hap_set_debug_level),
    ESP_ELFSYM_EXPORT(hap_get_version),
    ESP_ELFSYM_EXPORT(hap_get_config),
    ESP_ELFSYM_EXPORT(hap_set_config),
//  ESP_ELFSYM_EXPORT(hap_update_config_number),
    ESP_ELFSYM_EXPORT(hap_init),
//  ESP_ELFSYM_EXPORT(hap_deinit),
    ESP_ELFSYM_EXPORT(hap_start),
//  ESP_ELFSYM_EXPORT(hap_stop),
    ESP_ELFSYM_EXPORT(hap_acc_create),
//  ESP_ELFSYM_EXPORT(hap_acc_add_accessory_flags),
//  ESP_ELFSYM_EXPORT(hap_acc_update_accessory_flags),
//  ESP_ELFSYM_EXPORT(hap_acc_add_product_data),
//  ESP_ELFSYM_EXPORT(hap_acc_add_wifi_transport_service),
    ESP_ELFSYM_EXPORT(hap_add_accessory),
//  ESP_ELFSYM_EXPORT(hap_add_bridged_accessory),
//  ESP_ELFSYM_EXPORT(hap_remove_bridged_accessory),
//  ESP_ELFSYM_EXPORT(hap_acc_delete),
//  ESP_ELFSYM_EXPORT(hap_delete_all_accessories),
//  ESP_ELFSYM_EXPORT(hap_get_unique_aid),
//  ESP_ELFSYM_EXPORT(hap_acc_get_by_aid),
//  ESP_ELFSYM_EXPORT(hap_get_first_acc),
//  ESP_ELFSYM_EXPORT(hap_acc_get_next),
//  ESP_ELFSYM_EXPORT(hap_acc_get_serv_by_iid),
//  ESP_ELFSYM_EXPORT(hap_acc_get_serv_by_uuid),
//  ESP_ELFSYM_EXPORT(hap_acc_get_char_by_iid),
//  ESP_ELFSYM_EXPORT(hap_acc_get_first_serv),
//  ESP_ELFSYM_EXPORT(hap_char_bool_create),
//  ESP_ELFSYM_EXPORT(hap_char_uint8_create),
//  ESP_ELFSYM_EXPORT(hap_char_uint16_create),
//  ESP_ELFSYM_EXPORT(hap_char_uint32_create),
//  ESP_ELFSYM_EXPORT(hap_char_uint64_create),
//  ESP_ELFSYM_EXPORT(hap_char_int_create),
//  ESP_ELFSYM_EXPORT(hap_char_float_create),
//  ESP_ELFSYM_EXPORT(hap_char_string_create),
//  ESP_ELFSYM_EXPORT(hap_char_data_create),
//  ESP_ELFSYM_EXPORT(hap_char_tlv8_create),
//  ESP_ELFSYM_EXPORT(hap_char_delete),
//  ESP_ELFSYM_EXPORT(hap_serv_create),
//  ESP_ELFSYM_EXPORT(hap_serv_delete),
//  ESP_ELFSYM_EXPORT(hap_acc_get_aid),
//  ESP_ELFSYM_EXPORT(hap_serv_get_char_by_uuid),
    ESP_ELFSYM_EXPORT(hap_serv_get_parent),
//  ESP_ELFSYM_EXPORT(hap_serv_get_next),
//  ESP_ELFSYM_EXPORT(hap_serv_get_first_char),
//  ESP_ELFSYM_EXPORT(hap_char_get_iid),
    ESP_ELFSYM_EXPORT(hap_char_get_type_uuid),
//  ESP_ELFSYM_EXPORT(hap_char_get_perm),
//  ESP_ELFSYM_EXPORT(hap_char_get_format),
//  ESP_ELFSYM_EXPORT(hap_serv_get_iid),
    ESP_ELFSYM_EXPORT(hap_serv_get_type_uuid),
    ESP_ELFSYM_EXPORT(hap_char_get_parent),
//  ESP_ELFSYM_EXPORT(hap_char_get_next),
//  ESP_ELFSYM_EXPORT(hap_char_int_set_constraints),
//  ESP_ELFSYM_EXPORT(hap_char_float_set_constraints),
//  ESP_ELFSYM_EXPORT(hap_char_string_set_maxlen),
//  ESP_ELFSYM_EXPORT(hap_char_add_description),
//  ESP_ELFSYM_EXPORT(hap_char_add_unit),
//  ESP_ELFSYM_EXPORT(hap_char_add_valid_vals),
//  ESP_ELFSYM_EXPORT(hap_char_add_valid_vals_range),
//  ESP_ELFSYM_EXPORT(hap_char_set_iid),
    ESP_ELFSYM_EXPORT(hap_serv_add_char),
    ESP_ELFSYM_EXPORT(hap_acc_add_serv),
    ESP_ELFSYM_EXPORT(hap_char_update_val),
//  ESP_ELFSYM_EXPORT(hap_char_get_val),
    ESP_ELFSYM_EXPORT(hap_serv_set_write_cb),
    ESP_ELFSYM_EXPORT(hap_serv_set_read_cb),
//  ESP_ELFSYM_EXPORT(hap_serv_set_bulk_read_cb),
//  ESP_ELFSYM_EXPORT(hap_serv_set_priv),
//  ESP_ELFSYM_EXPORT(hap_serv_get_priv),
//  ESP_ELFSYM_EXPORT(hap_serv_mark_primary),
//  ESP_ELFSYM_EXPORT(hap_serv_mark_hidden),
//  ESP_ELFSYM_EXPORT(hap_serv_link_serv),
//  ESP_ELFSYM_EXPORT(hap_serv_set_iid),
//  ESP_ELFSYM_EXPORT(hap_set_setup_info),
    ESP_ELFSYM_EXPORT(hap_set_setup_code),
    ESP_ELFSYM_EXPORT(hap_set_setup_id),
#if 0
    ESP_ELFSYM_EXPORT(hap_check_mfi_chip),
#endif
    ESP_ELFSYM_EXPORT(hap_reboot_accessory),
    ESP_ELFSYM_EXPORT(hap_reset_to_factory),
    ESP_ELFSYM_EXPORT(hap_reset_network),
    ESP_ELFSYM_EXPORT(hap_reset_homekit_data),
    ESP_ELFSYM_EXPORT(hap_reset_pairings),
//  ESP_ELFSYM_EXPORT(hap_is_req_admin),
    ESP_ELFSYM_EXPORT(hap_req_get_ctrl_id),
//  ESP_ELFSYM_EXPORT(hap_factory_keystore_get),
//  ESP_ELFSYM_EXPORT(hap_enable_mfi_auth),
//  ESP_ELFSYM_EXPORT(hap_register_event_handler),
    ESP_ELFSYM_EXPORT(hap_get_paired_controller_count),
//  ESP_ELFSYM_EXPORT(hap_http_debug_enable),
//  ESP_ELFSYM_EXPORT(hap_http_debug_disable),
//  ESP_ELFSYM_EXPORT(esp_hap_get_setup_payload),
//  ESP_ELFSYM_EXPORT(hap_pair_setup_re_enable),

    // hap_apple_chars
    ESP_ELFSYM_EXPORT(hap_char_brightness_create),
    ESP_ELFSYM_EXPORT(hap_char_cooling_threshold_temperature_create),
    ESP_ELFSYM_EXPORT(hap_char_current_door_state_create),
    ESP_ELFSYM_EXPORT(hap_char_current_heating_cooling_state_create),
    ESP_ELFSYM_EXPORT(hap_char_current_relative_humidity_create),
    ESP_ELFSYM_EXPORT(hap_char_current_temperature_create),
    ESP_ELFSYM_EXPORT(hap_char_firmware_revision_create),
    ESP_ELFSYM_EXPORT(hap_char_hardware_revision_create),
    ESP_ELFSYM_EXPORT(hap_char_heating_threshold_temperature_create),
    ESP_ELFSYM_EXPORT(hap_char_hue_create),
    ESP_ELFSYM_EXPORT(hap_char_identify_create),
    ESP_ELFSYM_EXPORT(hap_char_lock_current_state_create),
    ESP_ELFSYM_EXPORT(hap_char_lock_target_state_create),
    ESP_ELFSYM_EXPORT(hap_char_manufacturer_create),
    ESP_ELFSYM_EXPORT(hap_char_model_create),
    ESP_ELFSYM_EXPORT(hap_char_motion_detected_create),
    ESP_ELFSYM_EXPORT(hap_char_name_create),
    ESP_ELFSYM_EXPORT(hap_char_obstruction_detect_create),
    ESP_ELFSYM_EXPORT(hap_char_on_create),
    ESP_ELFSYM_EXPORT(hap_char_outlet_in_use_create),
    ESP_ELFSYM_EXPORT(hap_char_rotation_direction_create),
    ESP_ELFSYM_EXPORT(hap_char_rotation_speed_create),
    ESP_ELFSYM_EXPORT(hap_char_saturation_create),
    ESP_ELFSYM_EXPORT(hap_char_serial_number_create),
    ESP_ELFSYM_EXPORT(hap_char_target_door_state_create),
    ESP_ELFSYM_EXPORT(hap_char_target_heating_cooling_state_create),
    ESP_ELFSYM_EXPORT(hap_char_target_relative_humidity_create),
    ESP_ELFSYM_EXPORT(hap_char_target_temperature_create),
    ESP_ELFSYM_EXPORT(hap_char_temperature_display_units_create),
    ESP_ELFSYM_EXPORT(hap_char_version_create),
    ESP_ELFSYM_EXPORT(hap_char_security_system_current_state_create),
    ESP_ELFSYM_EXPORT(hap_char_security_system_target_state_create),
    ESP_ELFSYM_EXPORT(hap_char_battery_level_create),
    ESP_ELFSYM_EXPORT(hap_char_current_position_create),
    ESP_ELFSYM_EXPORT(hap_char_current_vertical_tilt_angle_create),
    ESP_ELFSYM_EXPORT(hap_char_hold_position_create),
    ESP_ELFSYM_EXPORT(hap_char_leak_detected_create),
    ESP_ELFSYM_EXPORT(hap_char_occupancy_detected_create),
    ESP_ELFSYM_EXPORT(hap_char_status_active_create),
    ESP_ELFSYM_EXPORT(hap_char_smoke_detected_create),
    ESP_ELFSYM_EXPORT(hap_char_status_fault_create),
    ESP_ELFSYM_EXPORT(hap_char_status_low_battery_create),
    ESP_ELFSYM_EXPORT(hap_char_status_tampered_create),
    ESP_ELFSYM_EXPORT(hap_char_target_horizontal_tilt_angle_create),
    ESP_ELFSYM_EXPORT(hap_char_target_position_create),
    ESP_ELFSYM_EXPORT(hap_char_target_vertical_tilt_angle_create),
    ESP_ELFSYM_EXPORT(hap_char_security_system_alarm_type_create),
    ESP_ELFSYM_EXPORT(hap_char_charging_state_create),
    ESP_ELFSYM_EXPORT(hap_char_carbon_monoxide_level_create),
    ESP_ELFSYM_EXPORT(hap_char_carbon_monoxide_peak_level_create),
    ESP_ELFSYM_EXPORT(hap_char_carbon_dioxide_detected_create),
    ESP_ELFSYM_EXPORT(hap_char_carbon_dioxide_level_create),
    ESP_ELFSYM_EXPORT(hap_char_carbon_dioxide_peak_level_create),
    ESP_ELFSYM_EXPORT(hap_char_carbon_monoxide_detected_create),
    ESP_ELFSYM_EXPORT(hap_char_contact_sensor_state_create),
    ESP_ELFSYM_EXPORT(hap_char_current_ambient_light_level_create),
    ESP_ELFSYM_EXPORT(hap_char_current_horizontal_tilt_angle_create),
    ESP_ELFSYM_EXPORT(hap_char_air_quality_create),
    ESP_ELFSYM_EXPORT(hap_char_accessory_flags_create),
    ESP_ELFSYM_EXPORT(hap_char_product_data_create),
    ESP_ELFSYM_EXPORT(hap_char_lock_physical_controls_create),
    ESP_ELFSYM_EXPORT(hap_char_current_air_purifier_state_create),
    ESP_ELFSYM_EXPORT(hap_char_current_slat_state_create),
    ESP_ELFSYM_EXPORT(hap_char_slat_type_create),
    ESP_ELFSYM_EXPORT(hap_char_filter_life_level_create),
    ESP_ELFSYM_EXPORT(hap_char_filter_change_indication_create),
    ESP_ELFSYM_EXPORT(hap_char_reset_filter_indication_create),
    ESP_ELFSYM_EXPORT(hap_char_target_air_purifier_state_create),
    ESP_ELFSYM_EXPORT(hap_char_target_fan_state_create),
    ESP_ELFSYM_EXPORT(hap_char_current_fan_state_create),
    ESP_ELFSYM_EXPORT(hap_char_position_state_create),
    ESP_ELFSYM_EXPORT(hap_char_programmable_switch_event_create),
    ESP_ELFSYM_EXPORT(hap_char_active_create),
    ESP_ELFSYM_EXPORT(hap_char_swing_mode_create),
    ESP_ELFSYM_EXPORT(hap_char_current_tilt_angle_create),
    ESP_ELFSYM_EXPORT(hap_char_target_tilt_angle_create),
    ESP_ELFSYM_EXPORT(hap_char_ozone_density_create),
    ESP_ELFSYM_EXPORT(hap_char_nitrogen_dioxide_density_create),
    ESP_ELFSYM_EXPORT(hap_char_sulphur_dioxide_density_create),
    ESP_ELFSYM_EXPORT(hap_char_pm_2_5_density_create),
    ESP_ELFSYM_EXPORT(hap_char_pm_10_density_create),
    ESP_ELFSYM_EXPORT(hap_char_voc_density_create),
    ESP_ELFSYM_EXPORT(hap_char_service_label_index_create),
    ESP_ELFSYM_EXPORT(hap_char_service_label_namespace_create),
    ESP_ELFSYM_EXPORT(hap_char_color_temperature_create),
    ESP_ELFSYM_EXPORT(hap_char_current_heater_cooler_state_create),
    ESP_ELFSYM_EXPORT(hap_char_target_heater_cooler_state_create),
    ESP_ELFSYM_EXPORT(hap_char_current_humidifier_dehumidifier_state_create),
    ESP_ELFSYM_EXPORT(hap_char_target_humidifier_dehumidifier_state_create),
    ESP_ELFSYM_EXPORT(hap_char_water_level_create),
    ESP_ELFSYM_EXPORT(hap_char_relative_humidity_dehumidifier_threshold_create),
    ESP_ELFSYM_EXPORT(hap_char_relative_humidity_humidifier_threshold_create),
    ESP_ELFSYM_EXPORT(hap_char_program_mode_create),
    ESP_ELFSYM_EXPORT(hap_char_in_use_create),
    ESP_ELFSYM_EXPORT(hap_char_set_duration_create),
    ESP_ELFSYM_EXPORT(hap_char_remaining_duration_create),
    ESP_ELFSYM_EXPORT(hap_char_valve_type_create),
    ESP_ELFSYM_EXPORT(hap_char_is_configured_create),
    ESP_ELFSYM_EXPORT(hap_char_status_jammed_create),
    ESP_ELFSYM_EXPORT(hap_char_administrator_only_access_create),
    ESP_ELFSYM_EXPORT(hap_char_lock_control_point_create),
    ESP_ELFSYM_EXPORT(hap_char_lock_last_known_action_create),
    ESP_ELFSYM_EXPORT(hap_char_lock_management_auto_security_timeout_create),
    ESP_ELFSYM_EXPORT(hap_char_logs_create),
    ESP_ELFSYM_EXPORT(hap_char_air_particulate_density_create),
    ESP_ELFSYM_EXPORT(hap_char_air_particulate_size_create),

    // hap_apple_servs
    ESP_ELFSYM_EXPORT(hap_serv_accessory_information_create),
    ESP_ELFSYM_EXPORT(hap_serv_protocol_information_create),
    ESP_ELFSYM_EXPORT(hap_serv_fan_create),
    ESP_ELFSYM_EXPORT(hap_serv_garage_door_opener_create),
    ESP_ELFSYM_EXPORT(hap_serv_lightbulb_create),
    ESP_ELFSYM_EXPORT(hap_serv_lock_management_create),
    ESP_ELFSYM_EXPORT(hap_serv_lock_mechanism_create),
    ESP_ELFSYM_EXPORT(hap_serv_outlet_create),
    ESP_ELFSYM_EXPORT(hap_serv_switch_create),
    ESP_ELFSYM_EXPORT(hap_serv_thermostat_create),
    ESP_ELFSYM_EXPORT(hap_serv_air_quality_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_security_system_create),
    ESP_ELFSYM_EXPORT(hap_serv_carbon_monoxide_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_contact_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_door_create),
    ESP_ELFSYM_EXPORT(hap_serv_humidity_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_leak_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_light_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_motion_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_occupancy_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_smoke_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_stateless_programmable_switch_create),
    ESP_ELFSYM_EXPORT(hap_serv_temperature_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_window_create),
    ESP_ELFSYM_EXPORT(hap_serv_window_covering_create),
    ESP_ELFSYM_EXPORT(hap_serv_battery_service_create),
    ESP_ELFSYM_EXPORT(hap_serv_carbon_dioxide_sensor_create),
    ESP_ELFSYM_EXPORT(hap_serv_fan_v2_create),
    ESP_ELFSYM_EXPORT(hap_serv_slat_create),
    ESP_ELFSYM_EXPORT(hap_serv_filter_maintenance_create),
    ESP_ELFSYM_EXPORT(hap_serv_air_purifier_create),
    ESP_ELFSYM_EXPORT(hap_serv_heater_cooler_create),
    ESP_ELFSYM_EXPORT(hap_serv_humidifier_dehumidifier_create),
    ESP_ELFSYM_EXPORT(hap_serv_service_label_create),
    ESP_ELFSYM_EXPORT(hap_serv_irrigation_system_create),
    ESP_ELFSYM_EXPORT(hap_serv_valve_create),
    ESP_ELFSYM_EXPORT(hap_serv_faucet_create),

    // hap_controllers
    ESP_ELFSYM_EXPORT(is_accessory_paired),
    ESP_ELFSYM_EXPORT(is_admin_paired),

    // hap_database
    ESP_ELFSYM_EXPORT(hap_get_acc_id),

    // end
    ESP_ELFSYM_END
};

#include "esp32c3.h"
#include "elf_loader/include/private/elf_symbol.h"

const struct esp_elfsym g_customer_elfsyms[] = {

    // esp_event
    ESP_ELFSYM_EXPORT(esp_event_loop_create),
    ESP_ELFSYM_EXPORT(esp_event_loop_delete),
    ESP_ELFSYM_EXPORT(esp_event_loop_create_default),
    ESP_ELFSYM_EXPORT(esp_event_loop_delete_default),
    ESP_ELFSYM_EXPORT(esp_event_loop_run),
    ESP_ELFSYM_EXPORT(esp_event_handler_register),
    ESP_ELFSYM_EXPORT(esp_event_handler_register_with),
    ESP_ELFSYM_EXPORT(esp_event_handler_instance_register_with),
    ESP_ELFSYM_EXPORT(esp_event_handler_instance_register),
    ESP_ELFSYM_EXPORT(esp_event_handler_unregister),
    ESP_ELFSYM_EXPORT(esp_event_handler_unregister_with),
    ESP_ELFSYM_EXPORT(esp_event_handler_instance_unregister_with),
    ESP_ELFSYM_EXPORT(esp_event_handler_instance_unregister),
    ESP_ELFSYM_EXPORT(esp_event_post),
    ESP_ELFSYM_EXPORT(esp_event_post_to),
#if CONFIG_ESP_EVENT_POST_FROM_ISR
    ESP_ELFSYM_EXPORT(esp_event_isr_post),
    ESP_ELFSYM_EXPORT(esp_event_isr_post_to),
#endif
    ESP_ELFSYM_EXPORT(esp_event_dump),

    // esp_netif
    ESP_ELFSYM_EXPORT(esp_netif_init),
    ESP_ELFSYM_EXPORT(esp_netif_deinit),
    ESP_ELFSYM_EXPORT(esp_netif_new),
    ESP_ELFSYM_EXPORT(esp_netif_destroy),
    ESP_ELFSYM_EXPORT(esp_netif_set_driver_config),
    ESP_ELFSYM_EXPORT(esp_netif_attach),
    ESP_ELFSYM_EXPORT(esp_netif_receive),
    ESP_ELFSYM_EXPORT(esp_netif_action_start),
    ESP_ELFSYM_EXPORT(esp_netif_action_stop),
    ESP_ELFSYM_EXPORT(esp_netif_action_connected),
    ESP_ELFSYM_EXPORT(esp_netif_action_disconnected),
    ESP_ELFSYM_EXPORT(esp_netif_action_got_ip),
    ESP_ELFSYM_EXPORT(esp_netif_action_join_ip6_multicast_group),
    ESP_ELFSYM_EXPORT(esp_netif_action_leave_ip6_multicast_group),
    ESP_ELFSYM_EXPORT(esp_netif_action_add_ip6_address),
    ESP_ELFSYM_EXPORT(esp_netif_action_remove_ip6_address),
    ESP_ELFSYM_EXPORT(esp_netif_set_default_netif),
    ESP_ELFSYM_EXPORT(esp_netif_get_default_netif),
#if CONFIG_ESP_NETIF_BRIDGE_EN
    ESP_ELFSYM_EXPORT(esp_netif_bridge_add_port),
    ESP_ELFSYM_EXPORT(esp_netif_bridge_fdb_add),
    ESP_ELFSYM_EXPORT(esp_netif_bridge_fdb_remove),
#endif
    ESP_ELFSYM_EXPORT(esp_netif_join_ip6_multicast_group),
    ESP_ELFSYM_EXPORT(esp_netif_leave_ip6_multicast_group),
    ESP_ELFSYM_EXPORT(esp_netif_set_mac),
    ESP_ELFSYM_EXPORT(esp_netif_get_mac),
    ESP_ELFSYM_EXPORT(esp_netif_set_hostname),
    ESP_ELFSYM_EXPORT(esp_netif_get_hostname),
    ESP_ELFSYM_EXPORT(esp_netif_is_netif_up),
    ESP_ELFSYM_EXPORT(esp_netif_get_ip_info),
    ESP_ELFSYM_EXPORT(esp_netif_get_old_ip_info),
    ESP_ELFSYM_EXPORT(esp_netif_set_ip_info),
    ESP_ELFSYM_EXPORT(esp_netif_set_old_ip_info),
    ESP_ELFSYM_EXPORT(esp_netif_get_netif_impl_index),
    ESP_ELFSYM_EXPORT(esp_netif_get_netif_impl_name),
    ESP_ELFSYM_EXPORT(esp_netif_napt_enable),
    ESP_ELFSYM_EXPORT(esp_netif_napt_disable),
    ESP_ELFSYM_EXPORT(esp_netif_dhcps_option),
    ESP_ELFSYM_EXPORT(esp_netif_dhcpc_option),
    ESP_ELFSYM_EXPORT(esp_netif_dhcpc_start),
    ESP_ELFSYM_EXPORT(esp_netif_dhcpc_stop),
    ESP_ELFSYM_EXPORT(esp_netif_dhcpc_get_status),
    ESP_ELFSYM_EXPORT(esp_netif_dhcps_get_status),
    ESP_ELFSYM_EXPORT(esp_netif_dhcps_start),
    ESP_ELFSYM_EXPORT(esp_netif_dhcps_stop),
    ESP_ELFSYM_EXPORT(esp_netif_dhcps_get_clients_by_mac),
    ESP_ELFSYM_EXPORT(esp_netif_set_dns_info),
    ESP_ELFSYM_EXPORT(esp_netif_get_dns_info),
#if CONFIG_LWIP_IPV6
    ESP_ELFSYM_EXPORT(esp_netif_create_ip6_linklocal),
    ESP_ELFSYM_EXPORT(esp_netif_get_ip6_linklocal),
    ESP_ELFSYM_EXPORT(esp_netif_get_ip6_global),
    ESP_ELFSYM_EXPORT(esp_netif_get_all_ip6),
    ESP_ELFSYM_EXPORT(esp_netif_add_ip6_address),
    ESP_ELFSYM_EXPORT(esp_netif_remove_ip6_address),
#endif
    ESP_ELFSYM_EXPORT(esp_netif_set_ip4_addr),
    ESP_ELFSYM_EXPORT(esp_ip4addr_ntoa),
    ESP_ELFSYM_EXPORT(esp_ip4addr_aton),
    ESP_ELFSYM_EXPORT(esp_netif_str_to_ip4),
    ESP_ELFSYM_EXPORT(esp_netif_str_to_ip6),
    ESP_ELFSYM_EXPORT(esp_netif_get_io_driver),
    ESP_ELFSYM_EXPORT(esp_netif_get_handle_from_ifkey),
    ESP_ELFSYM_EXPORT(esp_netif_get_flags),
    ESP_ELFSYM_EXPORT(esp_netif_get_ifkey),
    ESP_ELFSYM_EXPORT(esp_netif_get_desc),
    ESP_ELFSYM_EXPORT(esp_netif_get_route_prio),
    ESP_ELFSYM_EXPORT(esp_netif_get_event_id),
    ESP_ELFSYM_EXPORT(esp_netif_next_unsafe),
    ESP_ELFSYM_EXPORT(esp_netif_find_if),
    ESP_ELFSYM_EXPORT(esp_netif_get_nr_of_ifs),
    ESP_ELFSYM_EXPORT(esp_netif_netstack_buf_ref),
    ESP_ELFSYM_EXPORT(esp_netif_netstack_buf_free),
    ESP_ELFSYM_EXPORT(esp_netif_tcpip_exec),

    // esp_wifi
    ESP_ELFSYM_EXPORT(esp_wifi_init),
    ESP_ELFSYM_EXPORT(esp_wifi_deinit),
    ESP_ELFSYM_EXPORT(esp_wifi_set_mode),
    ESP_ELFSYM_EXPORT(esp_wifi_get_mode),
    ESP_ELFSYM_EXPORT(esp_wifi_start),
    ESP_ELFSYM_EXPORT(esp_wifi_stop),
    ESP_ELFSYM_EXPORT(esp_wifi_restore),
    ESP_ELFSYM_EXPORT(esp_wifi_connect),
    ESP_ELFSYM_EXPORT(esp_wifi_disconnect),
    ESP_ELFSYM_EXPORT(esp_wifi_clear_fast_connect),
    ESP_ELFSYM_EXPORT(esp_wifi_deauth_sta),
    ESP_ELFSYM_EXPORT(esp_wifi_scan_start),
    ESP_ELFSYM_EXPORT(esp_wifi_set_scan_parameters),
    ESP_ELFSYM_EXPORT(esp_wifi_get_scan_parameters),
    ESP_ELFSYM_EXPORT(esp_wifi_scan_stop),
    ESP_ELFSYM_EXPORT(esp_wifi_scan_get_ap_num),
    ESP_ELFSYM_EXPORT(esp_wifi_scan_get_ap_records),
    ESP_ELFSYM_EXPORT(esp_wifi_scan_get_ap_record),
    ESP_ELFSYM_EXPORT(esp_wifi_clear_ap_list),
    ESP_ELFSYM_EXPORT(esp_wifi_sta_get_ap_info),
    ESP_ELFSYM_EXPORT(esp_wifi_set_ps),
    ESP_ELFSYM_EXPORT(esp_wifi_get_ps),
    ESP_ELFSYM_EXPORT(esp_wifi_set_protocol),
    ESP_ELFSYM_EXPORT(esp_wifi_get_protocol),
    ESP_ELFSYM_EXPORT(esp_wifi_set_bandwidth),
    ESP_ELFSYM_EXPORT(esp_wifi_get_bandwidth),
    ESP_ELFSYM_EXPORT(esp_wifi_set_channel),
    ESP_ELFSYM_EXPORT(esp_wifi_get_channel),
    ESP_ELFSYM_EXPORT(esp_wifi_set_country),
    ESP_ELFSYM_EXPORT(esp_wifi_get_country),
    ESP_ELFSYM_EXPORT(esp_wifi_set_mac),
    ESP_ELFSYM_EXPORT(esp_wifi_get_mac),
    ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous_rx_cb),
    ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous),
    ESP_ELFSYM_EXPORT(esp_wifi_get_promiscuous),
    ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous_filter),
    ESP_ELFSYM_EXPORT(esp_wifi_get_promiscuous_filter),
    ESP_ELFSYM_EXPORT(esp_wifi_set_promiscuous_ctrl_filter),
    ESP_ELFSYM_EXPORT(esp_wifi_get_promiscuous_ctrl_filter),
    ESP_ELFSYM_EXPORT(esp_wifi_set_config),
    ESP_ELFSYM_EXPORT(esp_wifi_get_config),
    ESP_ELFSYM_EXPORT(esp_wifi_ap_get_sta_list),
    ESP_ELFSYM_EXPORT(esp_wifi_ap_get_sta_aid),
    ESP_ELFSYM_EXPORT(esp_wifi_set_storage),
    ESP_ELFSYM_EXPORT(esp_wifi_set_vendor_ie),
    ESP_ELFSYM_EXPORT(esp_wifi_set_vendor_ie_cb),
    ESP_ELFSYM_EXPORT(esp_wifi_set_max_tx_power),
    ESP_ELFSYM_EXPORT(esp_wifi_get_max_tx_power),
    ESP_ELFSYM_EXPORT(esp_wifi_set_event_mask),
    ESP_ELFSYM_EXPORT(esp_wifi_get_event_mask),
    ESP_ELFSYM_EXPORT(esp_wifi_80211_tx),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_csi_rx_cb),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_csi_config),
//  ESP_ELFSYM_EXPORT(esp_wifi_get_csi_config),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_csi),
    ESP_ELFSYM_EXPORT(esp_wifi_get_tsf_time),
    ESP_ELFSYM_EXPORT(esp_wifi_set_inactive_time),
    ESP_ELFSYM_EXPORT(esp_wifi_statis_dump),
    ESP_ELFSYM_EXPORT(esp_wifi_set_rssi_threshold),
    ESP_ELFSYM_EXPORT(esp_wifi_ftm_initiate_session),
    ESP_ELFSYM_EXPORT(esp_wifi_ftm_end_session),
    ESP_ELFSYM_EXPORT(esp_wifi_ftm_resp_set_offset),
    ESP_ELFSYM_EXPORT(esp_wifi_ftm_get_report),
    ESP_ELFSYM_EXPORT(esp_wifi_config_11b_rate),
    ESP_ELFSYM_EXPORT(esp_wifi_connectionless_module_set_wake_interval),
    ESP_ELFSYM_EXPORT(esp_wifi_force_wakeup_acquire),
    ESP_ELFSYM_EXPORT(esp_wifi_force_wakeup_release),
    ESP_ELFSYM_EXPORT(esp_wifi_set_country_code),
    ESP_ELFSYM_EXPORT(esp_wifi_get_country_code),
    ESP_ELFSYM_EXPORT(esp_wifi_config_80211_tx_rate),
    ESP_ELFSYM_EXPORT(esp_wifi_disable_pmf_config),
    ESP_ELFSYM_EXPORT(esp_wifi_sta_get_aid),
    ESP_ELFSYM_EXPORT(esp_wifi_sta_get_negotiated_phymode),
    ESP_ELFSYM_EXPORT(esp_wifi_set_dynamic_cs),
    ESP_ELFSYM_EXPORT(esp_wifi_sta_get_rssi),

    // esp_wifi_default
    ESP_ELFSYM_EXPORT(esp_netif_attach_wifi_station),
    ESP_ELFSYM_EXPORT(esp_netif_attach_wifi_ap),
    ESP_ELFSYM_EXPORT(esp_wifi_set_default_wifi_sta_handlers),
    ESP_ELFSYM_EXPORT(esp_wifi_set_default_wifi_ap_handlers),
//  ESP_ELFSYM_EXPORT(esp_wifi_set_default_wifi_nan_handlers),
    ESP_ELFSYM_EXPORT(esp_wifi_clear_default_wifi_driver_and_handlers),
    ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_ap),
    ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_sta),
//  ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_nan),
    ESP_ELFSYM_EXPORT(esp_netif_destroy_default_wifi),
    ESP_ELFSYM_EXPORT(esp_netif_create_wifi),
    ESP_ELFSYM_EXPORT(esp_netif_create_default_wifi_mesh_netifs),

    // end
    ESP_ELFSYM_END
};
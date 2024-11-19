/**
 * ROM APIs
 */
PROVIDE ( esp_rom_crc32_le = 0x40000628 );
PROVIDE ( esp_rom_crc16_le = 0x40000630 );
PROVIDE ( esp_rom_crc8_le  = 0x40000638 );
PROVIDE ( esp_rom_crc32_be = 0x4000062c );
PROVIDE ( esp_rom_crc16_be = 0x40000634 );
PROVIDE ( esp_rom_crc8_be  = 0x4000063c );

PROVIDE ( esp_rom_gpio_pad_select_gpio    = 0x400005c8 );
PROVIDE ( esp_rom_gpio_pad_pullup_only    = 0x400005c4 );
PROVIDE ( esp_rom_gpio_pad_set_drv        = 0x400005cc );
PROVIDE ( esp_rom_gpio_pad_unhold         = 0x400005d0 );
PROVIDE ( esp_rom_gpio_connect_in_signal  = 0x400005a0 );
PROVIDE ( esp_rom_gpio_connect_out_signal = 0x400005a4 );

PROVIDE ( esp_rom_efuse_mac_address_crc8       = 0x40000640 );
PROVIDE ( esp_rom_efuse_get_flash_gpio_info    = 0x4000071c );
PROVIDE ( esp_rom_efuse_is_secure_boot_enabled = 0x400006f8 );
PROVIDE ( esp_rom_efuse_get_flash_wp_gpio      = 0x4000072c );

PROVIDE ( esp_rom_uart_flush_tx       = 0x400008d4 );
PROVIDE ( esp_rom_uart_tx_one_char    = 0x40000068 );
PROVIDE ( esp_rom_uart_tx_wait_idle   = 0x40000084 );
PROVIDE ( esp_rom_uart_rx_one_char    = 0x40000070 );
PROVIDE ( esp_rom_uart_rx_string      = 0x40000064 );

PROVIDE ( esp_rom_md5_init   = 0x40000614 );
PROVIDE ( esp_rom_md5_update = 0x40000618 );
PROVIDE ( esp_rom_md5_final  = 0x4000061c );

PROVIDE ( esp_rom_software_reset_system = 0x40000090 );
PROVIDE ( esp_rom_software_reset_cpu    = 0x40000094 );

PROVIDE ( esp_rom_printf   = 0x40000040 );
PROVIDE ( esp_rom_delay_us = 0x40000050 );
PROVIDE ( esp_rom_get_reset_reason = 0x40000018 );
PROVIDE ( esp_rom_route_intr_matrix = 0x400005f4 );
PROVIDE ( esp_rom_get_cpu_ticks_per_us = 0x40000584 );
PROVIDE ( esp_rom_set_cpu_ticks_per_us = 0x40000588 );

PROVIDE ( esp_rom_spiflash_clear_bp = 0x40000140 );
PROVIDE ( esp_rom_spiflash_write_enable = 0x40000210 );
PROVIDE ( esp_rom_spiflash_erase_area = 0x400001e4 );

PROVIDE ( esp_rom_spiflash_fix_dummylen = 0x40000220 );
PROVIDE ( esp_rom_spiflash_set_drvs = 0x40000218 );
PROVIDE ( esp_rom_spiflash_select_padsfunc = 0x40000214 );
PROVIDE ( esp_rom_spiflash_common_cmd = 0x400001f0 );

PROVIDE ( esp_rom_regi2c_read = 0x40001954 );
PROVIDE ( esp_rom_regi2c_read_mask = 0x40001958 );
PROVIDE ( esp_rom_regi2c_write = 0x4000195c );
PROVIDE ( esp_rom_regi2c_write_mask = 0x40001960 );

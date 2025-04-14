/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_system.h"
#include "lcd.h"

static esp_lcd_panel_handle_t panel_handle = NULL;

void lcd_init(int addr, int sda, int scl)
{
    i2c_master_bus_config_t i2c_bus_conf = {
        .i2c_port = -1,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle = NULL;
    if (i2c_new_master_bus(&i2c_bus_conf, &bus_handle) != ESP_OK)
        return;

    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = addr,
        .control_phase_bytes = 1, // According to SSD1306 datasheet
        .dc_bit_offset = 6,       // According to SSD1306 datasheet
        .lcd_cmd_bits = 8,        // According to SSD1306 datasheet
        .lcd_param_bits = 8,      // According to SSD1306 datasheet
        .scl_speed_hz = (400 * 1000),
    };

    esp_lcd_panel_io_handle_t io_handle = NULL;
    if (esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &io_handle) != ESP_OK) {
        i2c_del_master_bus(bus_handle);
        return;
    }

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .bits_per_pixel = 1,
    };
    if (esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle) != ESP_OK) {
        esp_lcd_panel_io_del(io_handle);
        i2c_del_master_bus(bus_handle);
        return;
    }
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    lcd_text("");
}

// https://github.com/filmote/Font3x5
static uint8_t const font_ascii[94 * 3] = {
    0b00000, 0b10111, 0b00000, // !
    0b00011, 0b00000, 0b00011, // "
    0b11111, 0b01010, 0b11111, // #
    0b01010, 0b11111, 0b01010, // $
    0b11001, 0b00100, 0b10011, // %
    0b10111, 0b01000, 0b10100, // &
    0b00000, 0b00011, 0b00000, // '
    0b00000, 0b01110, 0b10001, // )
    0b10001, 0b01110, 0b00000, // (
    0b01010, 0b00100, 0b01010, // *
    0b00100, 0b01110, 0b00100, // +
   0b100000, 0b10000, 0b00000, // ,
    0b00100, 0b00100, 0b00100, // -
    0b00000, 0b10000, 0b00000, // .
    0b01000, 0b00100, 0b00010, // /
    0b11111, 0b10001, 0b11111, // 0
    0b00000, 0b11111, 0b00000, // 1
    0b11101, 0b10101, 0b10111, // 2
    0b10001, 0b10101, 0b11111, // 3
    0b00111, 0b00100, 0b11111, // 4
    0b10111, 0b10101, 0b11101, // 5
    0b11111, 0b10101, 0b11101, // 6
    0b00001, 0b00001, 0b11111, // 7
    0b11111, 0b10101, 0b11111, // 8
    0b10111, 0b10101, 0b11111, // 9
    0b00000, 0b01010, 0b00000, // :
    0b10000, 0b01010, 0b00000, // ;
    0b00100, 0b01010, 0b10001, // <
    0b01010, 0b01010, 0b01010, // =
    0b10001, 0b01010, 0b00100, // >
    0b00001, 0b10101, 0b00011, // ?
    0b01110, 0b11111, 0b01110, // @
    0b11111, 0b00101, 0b11111, // A
    0b11111, 0b10101, 0b11011, // B
    0b11111, 0b10001, 0b10001, // C
    0b11111, 0b10001, 0b01110, // D
    0b11111, 0b10101, 0b10001, // E
    0b11111, 0b00101, 0b00001, // F
    0b11111, 0b10001, 0b11101, // G
    0b11111, 0b00100, 0b11111, // H
    0b00000, 0b11111, 0b00000, // I
    0b10000, 0b10000, 0b11111, // J
    0b11111, 0b00100, 0b11011, // K
    0b11111, 0b10000, 0b10000, // L
    0b11111, 0b00110, 0b11111, // M
    0b11111, 0b00001, 0b11111, // N
    0b11111, 0b10001, 0b11111, // O
    0b11111, 0b00101, 0b00111, // P
    0b11111,0b110001, 0b11111, // Q
    0b11111, 0b00101, 0b11011, // R
    0b10111, 0b10101, 0b11101, // S
    0b00001, 0b11111, 0b00001, // T
    0b11111, 0b10000, 0b11111, // U
    0b01111, 0b10000, 0b01111, // V
    0b11111, 0b01100, 0b11111, // W
    0b11011, 0b00100, 0b11011, // X
    0b00111, 0b11100, 0b00111, // Y
    0b11001, 0b10101, 0b10111, // Z
    0b11111, 0b10001, 0b10001, // [
    0b00010, 0b00100, 0b01000, // \.
    0b10001, 0b10001, 0b11111, // ]
    0b00010, 0b00001, 0b00010, // ^
    0b10000, 0b10000, 0b10000, // _
    0b00000, 0b00001, 0b00010, // `
    0b01100, 0b10010, 0b11110, // a
    0b11111, 0b10010, 0b01100, // b
    0b11110, 0b10010, 0b10010, // c
    0b01100, 0b10010, 0b11111, // d
    0b01100, 0b11010, 0b10100, // e
    0b00100, 0b11111, 0b00101, // f
   0b101110,0b101010, 0b11110, // g
    0b11111, 0b00010, 0b11100, // h
    0b00000, 0b11101, 0b00000, // i
   0b100000, 0b11101, 0b00000, // j
    0b11111, 0b00100, 0b11010, // k
    0b00001, 0b11111, 0b00000, // l
    0b11110, 0b00100, 0b11110, // m
    0b11110, 0b00010, 0b11110, // n
    0b11110, 0b10010, 0b11110, // o
   0b111110, 0b10010, 0b01100, // p
    0b01100, 0b10010,0b111110, // q
    0b11110, 0b00010, 0b00110, // r
    0b10100, 0b10010, 0b01010, // s
    0b00010, 0b01111, 0b10010, // t
    0b11110, 0b10000, 0b11110, // u
    0b01110, 0b10000, 0b01110, // v
    0b11110, 0b01000, 0b11110, // w
    0b11010, 0b00100, 0b11010, // x
   0b101110,0b101000, 0b11110, // y
    0b11010, 0b10010, 0b10110, // z
    0b00100, 0b11011, 0b10001, // {
    0b00000, 0b11011, 0b00000, // |
    0b10001, 0b11011, 0b00100, // }
    0b00010, 0b01010, 0b01000, // ~
};

void lcd_text(char const* text)
{
    if (panel_handle == NULL)
        return;
    int length = 128 * 64 / 8;
    uint8_t* ram = malloc(length);
    if (ram == NULL)
        return;
    memset(ram, 0 , length);
    int pos = 0;
    for (char c; (c = *text); text++) {
        uint8_t const* font = NULL;
        if (c == '\n') {
            pos = (pos + 128) & ~127;
            continue;
        }
        else if (c == '\r') {
            continue;
        }
        else if (c >= '!' && c <= '~') {
            font = &font_ascii[(c - '!') * 3];
        }
        int x = pos % 128;
        int y = pos / 128;
        int shift = y * 2 % 8;
        int bottom = y * 6 / 8 * 128;
        int top = bottom;
        if (shift) {
            if (top >= length)
                break;
            ram[top + x + 0] |= (font ? font[0] : 0) << (8 - shift);
            ram[top + x + 1] |= (font ? font[1] : 0) << (8 - shift);
            ram[top + x + 2] |= (font ? font[2] : 0) << (8 - shift);
//          ram[top + x + 3] |= 0;
            bottom += 128;
        }
        if (bottom >= length)
            break;
        ram[bottom + x + 0] = (font ? font[0] : 0) >> shift;
        ram[bottom + x + 1] = (font ? font[1] : 0) >> shift;
        ram[bottom + x + 2] = (font ? font[2] : 0) >> shift;
        ram[bottom + x + 3] = 0;
        pos += 4;
    }
    esp_lcd_panel_disp_on_off(panel_handle, true);
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 128, 64, ram);
    free(ram);
}

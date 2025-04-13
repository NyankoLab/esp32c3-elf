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
static uint8_t const font_question[3] = {
    0x00, 0x17, 0x00,
};

static uint8_t const font_dot[3] = {
    0X00, 0x10, 0x00,
};

static uint8_t const font_number[10 * 3] = {
    0x1F, 0x11, 0x1F,
    0x00, 0x1F, 0x00,
    0x1D, 0x15, 0x17,
    0x11, 0x15, 0x1F,
    0x07, 0x04, 0x1F,
    0x17, 0x15, 0x1D,
    0x1F, 0x15, 0x1D,
    0x01, 0x01, 0x1F,
    0x1F, 0x15, 0x1F,
    0x17, 0x15, 0x1F,
};

static uint8_t const font_upper[26 * 3] = {
    0x1F, 0x05, 0x1F,
    0x1F, 0x15, 0x1B,
    0x1F, 0x11, 0x11,
    0x1F, 0x11, 0x0E,
    0x1F, 0x15, 0x11,
    0x1F, 0x05, 0x01,
    0x1F, 0x11, 0x1D,
    0x1F, 0x04, 0x1F,
    0x00, 0x1F, 0x00,
    0x10, 0x10, 0x1F,
    0x1F, 0x04, 0x1B,
    0x1F, 0x10, 0x10,
    0x1F, 0x06, 0x1F,
    0x1F, 0x01, 0x1F,
    0x1F, 0x11, 0x1F,
    0x1F, 0x05, 0x07,
    0x1F, 0x31, 0x1F,
    0x1F, 0x05, 0x1B,
    0x17, 0x15, 0x1D,
    0x01, 0x1F, 0x01,
    0x1F, 0x10, 0x1F,
    0x0F, 0x10, 0x0F,
    0x1F, 0x0C, 0x1F,
    0x1B, 0x04, 0x1B,
    0x07, 0x1C, 0x07,
    0x19, 0x15, 0x13,
};

static uint8_t const font_lower[26 * 3] = {
    0x0C, 0x12, 0x1E,
    0x1F, 0x12, 0x0C,
    0x1E, 0x12, 0x12,
    0x0C, 0x12, 0x1F,
    0x0C, 0x1A, 0x14,
    0x04, 0x1F, 0x05,
    0x2E, 0x2A, 0x1E,
    0x1F, 0x02, 0x1C,
    0x00, 0x1D, 0x00,
    0x20, 0x1D, 0x00,
    0x1F, 0x04, 0x1A,
    0x01, 0x1F, 0x00,
    0x1E, 0x04, 0x1E,
    0x1E, 0x02, 0x1E,
    0x1E, 0x12, 0x1E,
    0x3E, 0x12, 0x0C,
    0x0C, 0x12, 0x3E,
    0x1E, 0x02, 0x06,
    0x14, 0x12, 0x0A,
    0x02, 0x0F, 0x12,
    0x1E, 0x10, 0x1E,
    0x0E, 0x10, 0x0E,
    0x1E, 0x08, 0x1E,
    0x1A, 0x04, 0x1A,
    0x2E, 0x28, 0x1E,
    0x1A, 0x12, 0x16,
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
        else if (c == '!') {
            font = font_question;
        }
        else if (c == '.') {
            font = font_dot;
        }
        else if (c >= '0' && c <= '9') {
            font = &font_number[(c - '0') * 3];
        }
        else if (c >= 'A' && c <= 'Z') {
            font = &font_upper[(c - 'A') * 3];
        }
        else if (c >= 'a' && c <= 'z') {
            font = &font_lower[(c - 'a') * 3];
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

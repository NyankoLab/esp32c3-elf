/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(int addr, int sda, int scl);
void lcd_text(char const* text);

#ifdef __cplusplus
}
#endif

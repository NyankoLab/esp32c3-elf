/*
 * SHA functions
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef SHA_H
#define SHA_H

#include <string.h>
#include <hal/sha_hal.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

void esp_sha_acquire_hardware(void);
void esp_sha_release_hardware(void);

#endif /* AES_H */

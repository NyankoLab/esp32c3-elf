/*
 * AES functions
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef AES_H
#define AES_H

#include <hal/aes_hal.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

void esp_aes_acquire_hardware(void);
void esp_aes_release_hardware(void);

#define AES_BLOCK_SIZE 16

static inline void xor_aes_block(void *dst, const void *src)
{
    u32 *d = (u32 *) dst;
    u32 *s = (u32 *) src;
    *d++ ^= *s++;
    *d++ ^= *s++;
    *d++ ^= *s++;
    *d++ ^= *s++;
}

#endif /* AES_H */

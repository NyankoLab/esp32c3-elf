/*
 * AES (Rijndael) cipher - decrypt
 *
 * Modifications to public domain implementation:
 * - cleanup
 * - use C pre-processor to make it easier to change S table access
 * - added option (AES_SMALL_TABLES) for reducing code size by about 8 kB at
 *   cost of reduced throughput (quite small difference on Pentium 4,
 *   10-25% when using -O1 or -O2 optimization)
 *
 * Copyright (c) 2003-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "aes.h"
#include <stdlib.h>


void * aes_decrypt_init(const u8 *key, size_t len)
{
    size_t *context = malloc(sizeof(size_t) + len);
    context[0] = len;
    memcpy(&context[1], key, len);
    return context;
}


int aes_decrypt(void *ctx, const u8 *crypt, u8 *plain)
{
    size_t *context = ctx;
    esp_aes_acquire_hardware();
    aes_hal_setkey((uint8_t *)&context[1], context[0], ESP_AES_DECRYPT);
    aes_hal_transform_block(crypt, plain);
    esp_aes_release_hardware();
    return 0;
}


void aes_decrypt_deinit(void *ctx)
{
    free(ctx);
}

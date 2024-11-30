/*
 * AES-128/192/256 CTR
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "aes.h"


/**
 * aes_ctr_encrypt - AES-128/192/256 CTR mode encryption
 * @key: Key for encryption (key_len bytes)
 * @key_len: Length of the key (16, 24, or 32 bytes)
 * @nonce: Nonce for counter mode (16 bytes)
 * @data: Data to encrypt in-place
 * @data_len: Length of data in bytes
 * Returns: 0 on success, Negative value on failure
 */
int __wrap_aes_ctr_encrypt(const u8 *key, size_t key_len, const u8 *nonce,
                           u8 *data, size_t data_len)
{
    size_t j, len, left = data_len;
    int i;
    u8 *pos = data;
    u8 counter[AES_BLOCK_SIZE], buf[AES_BLOCK_SIZE];

    esp_aes_acquire_hardware();
    aes_hal_setkey(key, key_len, ESP_AES_ENCRYPT);
    memcpy(counter, nonce, AES_BLOCK_SIZE);

    while (left > 0) {
        aes_hal_transform_block(counter, buf);

        len = (left < AES_BLOCK_SIZE) ? left : AES_BLOCK_SIZE;
        for (j = 0; j < len; j++)
            pos[j] ^= buf[j];
        pos += len;
        left -= len;

        for (i = AES_BLOCK_SIZE - 1; i >= 0; i--) {
            counter[i]++;
            if (counter[i])
                break;
        }
    }
    esp_aes_release_hardware();
    return 0;
}


/**
 * aes_128_ctr_encrypt - AES-128 CTR mode encryption
 * @key: Key for encryption (key_len bytes)
 * @nonce: Nonce for counter mode (16 bytes)
 * @data: Data to encrypt in-place
 * @data_len: Length of data in bytes
 * Returns: 0 on success, -1 on failure
 */
int __wrap_aes_128_ctr_encrypt(const u8 *key, const u8 *nonce,
                               u8 *data, size_t data_len)
{
    return __wrap_aes_ctr_encrypt(key, 16, nonce, data, data_len);
}

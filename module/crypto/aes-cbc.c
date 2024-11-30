/*
 * AES-128 CBC
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "aes.h"


/**
 * aes_128_cbc_encrypt - AES-128 CBC encryption
 * @key: Encryption key
 * @iv: Encryption IV for CBC mode (16 bytes)
 * @data: Data to encrypt in-place
 * @data_len: Length of data in bytes (must be divisible by 16)
 * Returns: 0 on success, -1 on failure
 */
int __wrap_aes_128_cbc_encrypt(const u8 *key, const u8 *iv, u8 *data, size_t data_len)
{
    uint32_t iv_words[4] = { iv[0], iv[1], iv[2], iv[3] };
    uint32_t *data_words = (uint32_t *)data;
    uint32_t temp[4];
    esp_aes_acquire_hardware();
    aes_hal_setkey(key, 16, ESP_AES_ENCRYPT);
    while (data_len > 0) {
        aes_hal_transform_block(data_words, temp);
        xor_aes_block(data_words, iv_words);
        iv_words[0] = temp[0];
        iv_words[1] = temp[1];
        iv_words[2] = temp[2];
        iv_words[3] = temp[3];
        data_words += 4;
        data_len -= 16;
    }
    esp_aes_release_hardware();
    return 0;
}


/**
 * aes_128_cbc_decrypt - AES-128 CBC decryption
 * @key: Decryption key
 * @iv: Decryption IV for CBC mode (16 bytes)
 * @data: Data to decrypt in-place
 * @data_len: Length of data in bytes (must be divisible by 16)
 * Returns: 0 on success, -1 on failure
 */
int __wrap_aes_128_cbc_decrypt(const u8 *key, const u8 *iv, u8 *data, size_t data_len)
{
    uint32_t iv_words[4] = { iv[0], iv[1], iv[2], iv[3] };
    uint32_t *data_words = (uint32_t *)data;
    uint32_t temp[4];
    esp_aes_acquire_hardware();
    aes_hal_setkey(key, 16, ESP_AES_DECRYPT);
    while (data_len > 0) {
        xor_aes_block(data_words, iv_words);
        aes_hal_transform_block(data_words, temp);
        iv_words[0] = temp[0];
        iv_words[1] = temp[1];
        iv_words[2] = temp[2];
        iv_words[3] = temp[3];
        data_words += 4;
        data_len -= 16;
    }
    esp_aes_release_hardware();
    return 0;
}

/*
 * Counter with CBC-MAC (CCM) with AES
 *
 * Copyright (c) 2010-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "aes.h"


static void aes_ccm_auth_start(size_t M, size_t L, const u8 *nonce,
                               const u8 *aad, size_t aad_len, size_t plain_len,
                               u8 *x)
{
    u8 aad_buf[2 * AES_BLOCK_SIZE];
    u8 b[AES_BLOCK_SIZE];

    /* Authentication */
    /* B_0: Flags | Nonce N | l(m) */
    b[0] = aad_len ? 0x40 : 0 /* Adata */;
    b[0] |= (((M - 2) / 2) /* M' */ << 3);
    b[0] |= (L - 1) /* L' */;
    memcpy(&b[1], nonce, 15 - L);
    aes_hal_transform_block(b, x); /* X_1 = E(K, B_0) */

    if (!aad_len)
        return;

    memcpy(aad_buf + 2, aad, aad_len);
    memset(aad_buf + 2 + aad_len, 0, sizeof(aad_buf) - 2 - aad_len);

    xor_aes_block(aad_buf, x);
    aes_hal_transform_block(aad_buf, x); /* X_2 = E(K, X_1 XOR B_1) */

    if (aad_len > AES_BLOCK_SIZE - 2) {
        xor_aes_block(&aad_buf[AES_BLOCK_SIZE], x);
        /* X_3 = E(K, X_2 XOR B_2) */
        aes_hal_transform_block(&aad_buf[AES_BLOCK_SIZE], x);
    }
}


static void aes_ccm_auth(const u8 *data, size_t len, u8 *x)
{
    size_t last = len % AES_BLOCK_SIZE;
    size_t i;

    for (i = 0; i < len / AES_BLOCK_SIZE; i++) {
        /* X_i+1 = E(K, X_i XOR B_i) */
        xor_aes_block(x, data);
        data += AES_BLOCK_SIZE;
        aes_hal_transform_block(x, x);
    }
    if (last) {
        /* XOR zero-padded last block */
        for (i = 0; i < last; i++)
            x[i] ^= *data++;
        aes_hal_transform_block(x, x);
    }
}


static void aes_ccm_encr_start(size_t L, const u8 *nonce, u8 *a)
{
    /* A_i = Flags | Nonce N | Counter i */
    a[0] = L - 1; /* Flags = L' */
    memcpy(&a[1], nonce, 15 - L);
}


static void aes_ccm_encr(size_t L, const u8 *in, size_t len, u8 *out, u8 *a)
{
    size_t last = len % AES_BLOCK_SIZE;
    size_t i;

    /* crypt = msg XOR (S_1 | S_2 | ... | S_n) */
    for (i = 1; i <= len / AES_BLOCK_SIZE; i++) {
        /* S_i = E(K, A_i) */
        aes_hal_transform_block(a, out);
        xor_aes_block(out, in);
        out += AES_BLOCK_SIZE;
        in += AES_BLOCK_SIZE;
    }
    if (last) {
        aes_hal_transform_block(a, out);
        /* XOR zero-padded last block */
        for (i = 0; i < last; i++)
            *out++ ^= *in++;
    }
}


static void aes_ccm_encr_auth(size_t M, u8 *x, u8 *a, u8 *auth)
{
    size_t i;
    u8 tmp[AES_BLOCK_SIZE];

    /* U = T XOR S_0; S_0 = E(K, A_0) */
    aes_hal_transform_block(a, tmp);
    for (i = 0; i < M; i++)
        auth[i] = x[i] ^ tmp[i];
}


static void aes_ccm_decr_auth(size_t M, u8 *a, const u8 *auth, u8 *t)
{
    size_t i;
    u8 tmp[AES_BLOCK_SIZE];

    /* U = T XOR S_0; S_0 = E(K, A_0) */
    aes_hal_transform_block(a, tmp);
    for (i = 0; i < M; i++)
        t[i] = auth[i] ^ tmp[i];
}


/* AES-CCM with fixed L=2 and aad_len <= 30 assumption */
int __wrap_aes_ccm_ae(const u8 *key, size_t key_len, const u8 *nonce,
                      size_t M, const u8 *plain, size_t plain_len,
                      const u8 *aad, size_t aad_len, u8 *crypt, u8 *auth)
{
    const size_t L = 2;
    u8 x[AES_BLOCK_SIZE], a[AES_BLOCK_SIZE];

    if (aad_len > 30 || M > AES_BLOCK_SIZE)
        return -1;

    esp_aes_acquire_hardware();
    aes_hal_setkey(key, key_len, ESP_AES_ENCRYPT);

	aes_ccm_auth_start(M, L, nonce, aad, aad_len, plain_len, x);
	aes_ccm_auth(plain, plain_len, x);

	/* Encryption */
	aes_ccm_encr_start(L, nonce, a);
	aes_ccm_encr(L, plain, plain_len, crypt, a);
	aes_ccm_encr_auth(M, x, a, auth);

    esp_aes_release_hardware();

    return 0;
}


/* AES-CCM with fixed L=2 and aad_len <= 30 assumption */
int __wrap_aes_ccm_ad(const u8 *key, size_t key_len, const u8 *nonce,
                      size_t M, const u8 *crypt, size_t crypt_len,
                      const u8 *aad, size_t aad_len, const u8 *auth, u8 *plain)
{
    const size_t L = 2;
    u8 x[AES_BLOCK_SIZE], a[AES_BLOCK_SIZE];
    u8 t[AES_BLOCK_SIZE];

    if (aad_len > 30 || M > AES_BLOCK_SIZE)
        return -1;

    esp_aes_acquire_hardware();
    aes_hal_setkey(key, key_len, ESP_AES_ENCRYPT);

    /* Decryption */
    aes_ccm_encr_start(L, nonce, a);
    aes_ccm_decr_auth(M, a, auth, t);

    /* plaintext = msg XOR (S_1 | S_2 | ... | S_n) */
    aes_ccm_encr(L, crypt, crypt_len, plain, a);

    aes_ccm_auth_start(M, L, nonce, aad, aad_len, crypt_len, x);
    aes_ccm_auth(plain, crypt_len, x);

    esp_aes_release_hardware();

    if (memcmp(x, t, M) != 0) {
        return -1;
    }

    return 0;
}

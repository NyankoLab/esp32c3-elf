/*
 * SHA1 hash implementation and interface functions
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "sha.h"
#include <crypto/sha1_i.h>
#include <utils/common.h>


/**
 * sha1_vector - SHA-1 hash for data vector
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash
 * Returns: 0 on success, -1 of failure
 */
int __wrap_sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
    u8 buffer[64];
    size_t i, total = 0;
    bool first_block = true;

    esp_sha_acquire_hardware();
    for (i = 0; i <= num_elem; i++) {
        size_t left = total % 64;
        size_t fill = 64 - left;

        if (i == num_elem) {
            memset(buffer + left, 0, fill);
            buffer[left] = 0x80;
            if (left >= 56) {
                sha_hal_hash_block(SHA1, buffer, 64 / 4, first_block);
                first_block = false;
                memset(buffer, 0, 56);
            }
            u32 high = (total >> 29);
            u32 low = (total << 3);
            WPA_PUT_BE32(buffer + 56, high);
            WPA_PUT_BE32(buffer + 60, low);
            sha_hal_hash_block(SHA1, buffer, 64 / 4, first_block);
            first_block = false;
            break;
        }

        const u8* data = addr[i];
        size_t length = len[i];
        total += length;

        if (left && length >= fill) {
            memcpy(buffer + left, data, fill);
            data += fill;
            length -= fill;
            left = 0;
            sha_hal_hash_block(SHA1, buffer, 64 / 4, first_block);
            first_block = false;
        }

        while (length >= 64) {
            sha_hal_hash_block(SHA1, data, 64 / 4, first_block);
            data += 64;
            length -= 64;
            first_block = false;
        }

        if (length > 0) {
            memcpy(buffer + left, data, length);
        }
    }
    sha_hal_read_digest(SHA1, mac);
    esp_sha_release_hardware();
    return 0;
}


/* Hash a single 512-bit block. This is the core of the algorithm. */

void __wrap_SHA1Transform(u32 state[5], const unsigned char buffer[64])
{
    esp_sha_acquire_hardware();
    sha_hal_write_digest(SHA1, state);
    sha_hal_hash_block(SHA1, buffer, 64 / 4, false);
    sha_hal_read_digest(SHA1, state);
    esp_sha_release_hardware();
}


/* SHA1Init - Initialize new context */

void __wrap_SHA1Init(struct SHA1Context* context)
{
    /* SHA1 initialization constants */
    context->state[0] = __builtin_bswap32(0x67452301);
    context->state[1] = __builtin_bswap32(0xEFCDAB89);
    context->state[2] = __builtin_bswap32(0x98BADCFE);
    context->state[3] = __builtin_bswap32(0x10325476);
    context->state[4] = __builtin_bswap32(0xC3D2E1F0);
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void __wrap_SHA1Update(struct SHA1Context* context, const void* _data, u32 len)
{
    u32 i, j;
    const unsigned char *data = _data;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3))
        context->count[1]++;
    context->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
        __wrap_SHA1Transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            __wrap_SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void __wrap_SHA1Final(unsigned char digest[20], struct SHA1Context* context)
{
    u32 i;
    unsigned char finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)
            ((context->count[(i >= 4 ? 0 : 1)] >>
              ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    __wrap_SHA1Update(context, (unsigned char *) "\200", 1);
    while ((context->count[0] & 504) != 448) {
        __wrap_SHA1Update(context, (unsigned char *) "\0", 1);
    }
    __wrap_SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
    memcpy(digest, context->state, 20);
}

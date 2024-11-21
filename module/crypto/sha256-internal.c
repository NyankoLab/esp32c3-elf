/*
 * SHA-256 hash implementation and interface functions
 * Copyright (c) 2003-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "sha.h"
#include <crypto/sha256_i.h>
#include <utils/common.h>


/**
 * sha256_vector - SHA256 hash for data vector
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash
 * Returns: 0 on success, -1 of failure
 */
int __wrap_sha256_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
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
                sha_hal_hash_block(SHA2_256, buffer, 64 / 4, first_block);
                first_block = false;
                memset(buffer, 0, 56);
            }
            u32 high = (total >> 29);
            u32 low = (total << 3);
            WPA_PUT_BE32(buffer + 56, high);
            WPA_PUT_BE32(buffer + 60, low);
            sha_hal_hash_block(SHA2_256, buffer, 64 / 4, first_block);
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
            sha_hal_hash_block(SHA2_256, buffer, 64 / 4, first_block);
            first_block = false;
        }

        while (length >= 64) {
            sha_hal_hash_block(SHA2_256, data, 64 / 4, first_block);
            data += 64;
            length -= 64;
            first_block = false;
        }

        if (length > 0) {
            memcpy(buffer + left, data, length);
        }
    }
    sha_hal_read_digest(SHA2_256, mac);
    esp_sha_release_hardware();
    return 0;
}


/* compress 512-bits */
int __wrap_sha256_compress(struct sha256_state *md, unsigned char *buf)
{
    esp_sha_acquire_hardware();
    sha_hal_write_digest(SHA2_256, md->state);
    sha_hal_hash_block(SHA2_256, buf, 64 / 4, false);
    sha_hal_read_digest(SHA2_256, md->state);
    esp_sha_release_hardware();
    return 0;
}


/* Initialize the hash state */
void __wrap_sha256_init(struct sha256_state *md)
{
    md->curlen = 0;
    md->length = 0;
    md->state[0] = __builtin_bswap32(0x6A09E667UL);
    md->state[1] = __builtin_bswap32(0xBB67AE85UL);
    md->state[2] = __builtin_bswap32(0x3C6EF372UL);
    md->state[3] = __builtin_bswap32(0xA54FF53AUL);
    md->state[4] = __builtin_bswap32(0x510E527FUL);
    md->state[5] = __builtin_bswap32(0x9B05688CUL);
    md->state[6] = __builtin_bswap32(0x1F83D9ABUL);
    md->state[7] = __builtin_bswap32(0x5BE0CD19UL);
}

/**
   Process a block of memory though the hash
   @param md     The hash state
   @param in     The data to hash
   @param inlen  The length of the data (octets)
   @return CRYPT_OK if successful
*/
int __wrap_sha256_process(struct sha256_state *md, const unsigned char *in,
                          unsigned long inlen)
{
    unsigned long n;

    if (md->curlen >= sizeof(md->buf))
        return -1;

    while (inlen > 0) {
        if (md->curlen == 0 && inlen >= SHA256_BLOCK_SIZE) {
            if (__wrap_sha256_compress(md, (unsigned char *) in) < 0)
                return -1;
            md->length += SHA256_BLOCK_SIZE * 8;
            in += SHA256_BLOCK_SIZE;
            inlen -= SHA256_BLOCK_SIZE;
        } else {
            n = SHA256_BLOCK_SIZE - md->curlen;
            if (n > inlen)
                n = inlen;
            memcpy(md->buf + md->curlen, in, n);
            md->curlen += n;
            in += n;
            inlen -= n;
            if (md->curlen == SHA256_BLOCK_SIZE) {
                if (__wrap_sha256_compress(md, md->buf) < 0)
                    return -1;
                md->length += 8 * SHA256_BLOCK_SIZE;
                md->curlen = 0;
            }
        }
    }

    return 0;
}


/**
   Terminate the hash to get the digest
   @param md  The hash state
   @param out [out] The destination of the hash (32 bytes)
   @return CRYPT_OK if successful
*/
int __wrap_sha256_done(struct sha256_state *md, unsigned char *out)
{
    if (md->curlen >= sizeof(md->buf))
        return -1;

    /* increase the length of the message */
    md->length += md->curlen * 8;

    /* append the '1' bit */
    md->buf[md->curlen++] = (unsigned char) 0x80;

    /* if the length is currently above 56 bytes we append zeros
     * then compress.  Then we can fall back to padding zeros and length
     * encoding like normal.
     */
    if (md->curlen > 56) {
        while (md->curlen < SHA256_BLOCK_SIZE) {
            md->buf[md->curlen++] = (unsigned char) 0;
        }
        __wrap_sha256_compress(md, md->buf);
        md->curlen = 0;
    }

    /* pad up to 56 bytes of zeroes */
    while (md->curlen < 56) {
        md->buf[md->curlen++] = (unsigned char) 0;
    }

    /* store length */
    WPA_PUT_BE64(md->buf + 56, md->length);
    __wrap_sha256_compress(md, md->buf);

    /* copy output */
    memcpy(out, md->state, 32);

    return 0;
}

/*
 * Big number math
 * Copyright (c) 2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "esp32c3.h"
#include <mbedtls/bignum.h>
#define ESP_PLATFORM
#include <utils/common.h>


/*
 * The current version is just a wrapper for mbedtls library, so
 * struct bignum is just typecast to mbedtls_mpi.
 */

/**
 * bignum_init - Allocate memory for bignum
 * Returns: Pointer to allocated bignum or %NULL on failure
 */
struct bignum * bignum_init(void)
{
    mbedtls_mpi *bn = os_zalloc(sizeof(mbedtls_mpi));
    if (bn == NULL)
        return NULL;
    mbedtls_mpi_init(bn);
    return (struct bignum *)bn;
}


/**
 * bignum_deinit - Free bignum
 * @n: Bignum from bignum_init()
 */
void bignum_deinit(struct bignum *n)
{
    mbedtls_mpi_free((mbedtls_mpi *)n);
    os_free((mbedtls_mpi *)n);
}


/**
 * bignum_get_unsigned_bin - Get length of bignum as an unsigned binary buffer
 * @n: Bignum from bignum_init()
 * Returns: Length of n if written to a binary buffer
 */
size_t bignum_get_unsigned_bin_len(struct bignum *n)
{
    return mbedtls_mpi_size((mbedtls_mpi *) n);
}


/**
 * bignum_get_unsigned_bin - Set binary buffer to unsigned bignum
 * @n: Bignum from bignum_init()
 * @buf: Buffer for the binary number
 * @len: Length of the buffer, can be %NULL if buffer is known to be long
 * enough. Set to used buffer length on success if not %NULL.
 * Returns: 0 on success, -1 on failure
 */
int bignum_get_unsigned_bin(const struct bignum *n, u8 *buf, size_t *len)
{
    size_t need = mbedtls_mpi_size((mbedtls_mpi *) n);
    if (len && need > *len) {
        *len = need;
        return -1;
    }
    if (len) {
        *len = need;
    }
    return mbedtls_mpi_write_binary((mbedtls_mpi *) n, buf, need);
}


/**
 * bignum_set_unsigned_bin - Set bignum based on unsigned binary buffer
 * @n: Bignum from bignum_init(); to be set to the given value
 * @buf: Buffer with unsigned binary value
 * @len: Length of buf in octets
 * Returns: 0 on success, -1 on failure
 */
int bignum_set_unsigned_bin(struct bignum *n, const u8 *buf, size_t len)
{
    return mbedtls_mpi_read_binary((mbedtls_mpi *) n, buf, len);
}


/**
 * bignum_cmp - Signed comparison
 * @a: Bignum from bignum_init()
 * @b: Bignum from bignum_init()
 * Returns: 0 on success, -1 on failure
 */
int bignum_cmp(const struct bignum *a, const struct bignum *b)
{
    return mbedtls_mpi_cmp_mpi((mbedtls_mpi *) a, (mbedtls_mpi *) b);
}


/**
 * bignum_cmp_d - Compare bignum to standard integer
 * @a: Bignum from bignum_init()
 * @b: Small integer
 * Returns: -1 if a < b, 0 if a == b, 1 if a > b
 */
int bignum_cmp_d(const struct bignum *a, unsigned long b)
{
    return mbedtls_mpi_cmp_int((mbedtls_mpi *) a, b);
}


/**
 * bignum_add - c = a + b
 * @a: Bignum from bignum_init()
 * @b: Bignum from bignum_init()
 * @c: Bignum from bignum_init(); used to store the result of a + b
 * Returns: 0 on success, -1 on failure
 */
int bignum_add(const struct bignum *a, const struct bignum *b, struct bignum *c)
{
    return mbedtls_mpi_add_mpi((mbedtls_mpi *) a, (mbedtls_mpi *) b, (mbedtls_mpi *) c);
}


/**
 * bignum_sub - c = a - b
 * @a: Bignum from bignum_init()
 * @b: Bignum from bignum_init()
 * @c: Bignum from bignum_init(); used to store the result of a - b
 * Returns: 0 on success, -1 on failure
 */
int bignum_sub(const struct bignum *a, const struct bignum *b, struct bignum *c)
{
    return mbedtls_mpi_sub_mpi((mbedtls_mpi *) a, (mbedtls_mpi *) b, (mbedtls_mpi *) c);
}


/**
 * bignum_mul - c = a * b
 * @a: Bignum from bignum_init()
 * @b: Bignum from bignum_init()
 * @c: Bignum from bignum_init(); used to store the result of a * b
 * Returns: 0 on success, -1 on failure
 */
int bignum_mul(const struct bignum *a, const struct bignum *b, struct bignum *c)
{
    return mbedtls_mpi_mul_mpi((mbedtls_mpi *) a, (mbedtls_mpi *) b, (mbedtls_mpi *) c);
}


/**
 * bignum_mulmod - d = a * b (mod c)
 * @a: Bignum from bignum_init()
 * @b: Bignum from bignum_init()
 * @c: Bignum from bignum_init(); modulus
 * @d: Bignum from bignum_init(); used to store the result of a * b (mod c)
 * Returns: 0 on success, -1 on failure
 */
int bignum_mulmod(const struct bignum *a, const struct bignum *b, const struct bignum *c, struct bignum *d)
{
    int esp_mpi_mul_mpi_mod(mbedtls_mpi *Z, const mbedtls_mpi *X, const mbedtls_mpi *Y, const mbedtls_mpi *M);
    return esp_mpi_mul_mpi_mod((mbedtls_mpi *) d, (mbedtls_mpi *) a, (mbedtls_mpi *) b, (mbedtls_mpi *) c);
}


/**
 * bignum_exptmod - Modular exponentiation: d = a^b (mod c)
 * @a: Bignum from bignum_init(); base
 * @b: Bignum from bignum_init(); exponent
 * @c: Bignum from bignum_init(); modulus
 * @d: Bignum from bignum_init(); used to store the result of a^b (mod c)
 * Returns: 0 on success, -1 on failure
 */
int bignum_exptmod(const struct bignum *a, const struct bignum *b, const struct bignum *c, struct bignum *d)
{
    return mbedtls_mpi_exp_mod((mbedtls_mpi *) d, (const mbedtls_mpi *) a, (const mbedtls_mpi *) b, (const mbedtls_mpi *) c, NULL);
}

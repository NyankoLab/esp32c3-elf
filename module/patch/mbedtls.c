#include <stddef.h>

void mbedtls_strerror(int errnum, char* buffer, size_t buflen)
{
    buffer[0] = 0;
}

const char* mbedtls_high_level_strerr(int error_code)
{
    return "";
}

const char* mbedtls_low_level_strerr(int error_code)
{
    return "";
}

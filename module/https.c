#include "esp32c3.h"
#include <sys/socket.h>
#include <lwip/dns.h>
#include "https.h"

#if defined(CONFIG_ESP_WIFI_MBEDTLS_CRYPTO)
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#else
#include <utils/common.h>
#include <crypto/tls.h>
#endif

#define TAG __FILE_NAME__

struct https_context
{
    void* other_context;
    void (*disconn)(void* arg);
    void (*recv)(void* arg, char* pusrdata, int length);
    char* host;
    char* path;
    char* attr;
    ip_addr_t ip;
    int socket;
    void* temp;
#if defined(CONFIG_ESP_WIFI_MBEDTLS_CRYPTO)
    mbedtls_net_context server_fd;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
#else
    void* tls;
    struct tls_connection* conn;
    int need_more_data;
#endif
};

static void https_handler(void* arg)
{
    struct https_context* context = arg;

    context->socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (context->socket < 0)
        goto final;

    struct sockaddr_in sockaddr = {};
    sockaddr.sin_len = sizeof(sockaddr);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(443);
    sockaddr.sin_addr.s_addr = context->ip.u_addr.ip4.addr;
    if (lwip_connect(context->socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
        goto final;
    context->temp = malloc(1536);
    if (context->temp == NULL)
        goto final;

#if defined(CONFIG_ESP_WIFI_MBEDTLS_CRYPTO)
    mbedtls_ctr_drbg_init(&context->ctr_drbg);
    mbedtls_net_init(&context->server_fd);
    mbedtls_ssl_init(&context->ssl);
    mbedtls_ssl_config_init(&context->conf);
    mbedtls_entropy_init(&context->entropy);
    if (mbedtls_ctr_drbg_seed(&context->ctr_drbg, mbedtls_entropy_func, &context->entropy, (uint8_t*)"", 1) != 0)
        goto final;
    if (mbedtls_ssl_config_defaults(&context->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0)
        goto final;
    mbedtls_ssl_conf_authmode(&context->conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&context->conf, mbedtls_ctr_drbg_random, &context->ctr_drbg);
    if (mbedtls_ssl_setup(&context->ssl, &context->conf) != 0)
        goto final;
    context->server_fd.fd = context->socket;
    mbedtls_ssl_set_bio(&context->ssl, &context->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    mbedtls_ssl_set_hostname(&context->ssl, "www.google.com");

    // Handshake
    for (;;)
    {
        vTaskDelay(1);
        int result = mbedtls_ssl_handshake(&context->ssl);
        if (result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;
        if (result < 0)
            goto final;
        if (result == 0)
            break;
    }
    snprintf(context->temp,
             1536,
             "GET /%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "%s%s"
             "\r\n", context->path, context->host, context->attr ? context->attr : "", context->attr ? "\r\n" : "");
    https_send(context, context->temp, strlen(context->temp));
#else
    context->tls = tls_init(NULL);
    if (context->tls == NULL)
        goto final;
    context->conn = tls_connection_init(context->tls);
    if (context->conn == NULL)
        goto final;

    // Hello
    struct wpabuf in;
    wpabuf_set(&in, NULL, 0);
    struct wpabuf* out = tls_connection_handshake2(context->tls, context->conn, &in, NULL, &context->need_more_data);

    if (out)
    {
        if (wpabuf_len(out) != 0)
        {
            lwip_send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
        }
        wpabuf_free(out);
    }

    // Handshake
    for (;;)
    {
        int length = lwip_recv(context->socket, context->temp, 1536, 0);
        if (length < 0)
            goto final;
        if (length == 0)
            goto final;

        struct wpabuf in;
        wpabuf_set(&in, context->temp, length);
        struct wpabuf* out = tls_connection_handshake2(context->tls, context->conn, &in, NULL, &context->need_more_data);

        if (out == NULL)
        {
            if (context->need_more_data == 0)
            {
                ESP_LOGE(TAG, "TLS handshake failed");
                goto final;
            }
        }
        else
        {
            if (tls_connection_get_failed(context->tls, context->conn))
            {
                ESP_LOGE(TAG, "TLS handshake failed");
                goto final;
            }
            if (tls_connection_established(context->tls, context->conn))
            {
                ESP_LOGI(TAG, "TLS handshake established");
                if (wpabuf_len(out) != 0)
                {
                    lwip_send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
                }
                wpabuf_free(out);

                snprintf(context->temp,
                         1536,
                         "GET /%s HTTP/1.1\r\n"
                         "Host: %s\r\n"
                         "%s%s"
                         "\r\n", context->path, context->host, context->attr ? context->attr : "", context->attr ? "\r\n" : "");
                https_send(context, context->temp, strlen(context->temp));
                break;
            }
            lwip_send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
            wpabuf_free(out);
        }
    }
#endif

    // Established
    for (;;)
    {
#if defined(CONFIG_ESP_WIFI_MBEDTLS_CRYPTO)
        vTaskDelay(1);
        int length = mbedtls_ssl_read(&context->ssl, context->temp, 1536);
        if (length == MBEDTLS_ERR_SSL_WANT_READ)
            continue;
        if (length < 0)
            goto final;
        if (length == 0)
            goto final;

        context->recv(context, (char*)context->temp, length);
#else
        int length = lwip_recv(context->socket, context->temp, 1536, 0);
        if (length < 0)
            goto final;
        if (length == 0)
            goto final;

        struct wpabuf in;
        wpabuf_set(&in, context->temp, length);
        struct wpabuf* out = tls_connection_decrypt(context->tls, context->conn, &in);
        if (out == NULL)
        {
            if (context->need_more_data == 0)
            {
                ESP_LOGE(TAG, "TLS decrypt failed");
                goto final;
            }
        }
        else
        {
            if (wpabuf_len(out) != 0)
            {
                context->recv(context, (char*)wpabuf_mhead_u8(out), wpabuf_len(out));
            }
            wpabuf_free(out);
        }
#endif
    }

final:
    https_disconnect(context);
    free(context);
    ESP_LOGI(TAG, "HTTPS disconnected");
    vTaskDelete(NULL);
}

static void dns_found(const char* name, const ip_addr_t* ip, void* arg)
{
    struct https_context* context = arg;

    if (ip)
    {
        context->ip = *ip;
        xTaskCreate(&https_handler, "https_handler", 6144, context, tskIDLE_PRIORITY, NULL);
        return;
    }

    https_disconnect(context);
    free(context);
    ESP_LOGE(TAG, "%s not found", name);
}

void https_connect(const char* url, const char* attr, void (*recv)(void* arg, char* pusrdata, int length), void (*disconn)(void* arg))
{
    char* buffer = strdup(url);
    if (buffer == NULL)
        return;

    char* token = buffer;
    char* https = strsep(&token, ":/");
    strsep(&token, ":/");
    strsep(&token, ":/");
    char* host = strsep(&token, ":/");
    char* path = token;

    if (strncmp(https, "https", 5) == 0)
    {
        struct https_context* context = calloc(1, sizeof(struct https_context));
        context->disconn = disconn;
        context->recv = recv;
        context->host = strdup(host);
        context->path = strdup(path);
        context->attr = attr ? strdup(attr) : NULL;
        context->socket = -1;
        if (dns_gethostbyname(context->host, &context->ip, dns_found, context) == 0)
        {
            dns_found(context->host, &context->ip, context);
        }
    }

    free(buffer);
}

void https_disconnect(void* arg)
{
    struct https_context* context = arg;

    if (context)
    {
        if (context->socket >= 0)
        {
            lwip_close(context->socket);
            context->socket = -1;
        }
        if (context->disconn)
        {
            context->disconn(arg);
            context->disconn = NULL;
        }
#if defined(CONFIG_ESP_WIFI_MBEDTLS_CRYPTO)
        mbedtls_ssl_close_notify(&context->ssl);
        mbedtls_net_free(&context->server_fd);
        mbedtls_ssl_free(&context->ssl);
        mbedtls_ssl_config_free(&context->conf);
        mbedtls_ctr_drbg_free(&context->ctr_drbg);
        mbedtls_entropy_free(&context->entropy);
#else
        if (context->tls)
        {
            if (context->conn)
            {
                tls_connection_deinit(context->tls, context->conn);
                context->conn = NULL;
            }
            tls_deinit(context->tls);
            context->tls = NULL;
        }
#endif
        free(context->path);
        free(context->host);
        free(context->attr);
        free(context->temp);
        context->path = NULL;
        context->host = NULL;
        context->attr = NULL;
        context->temp = NULL;
    }
}

void https_send(void* arg, const void* data, int length)
{
    struct https_context* context = arg;
#if defined(CONFIG_ESP_WIFI_MBEDTLS_CRYPTO)
    for (;;)
    {
        if (length == 0)
            break;
        vTaskDelay(1);
        int result = mbedtls_ssl_write(&context->ssl, data, length);
        if (result == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;
        if (result < 0)
            break;
        if (result == 0)
            break;
        data += result;
        length -= result;
    }
#else
    struct wpabuf in;
    wpabuf_set(&in, data, length);
    struct wpabuf* out = tls_connection_encrypt(context->tls, context->conn, &in);
    if (out)
    {
        lwip_send(context->socket, wpabuf_mhead_u8(out), wpabuf_len(out), 0);
        wpabuf_free(out);
    }
#endif
}

void https_callback(void* arg, void (*recv)(void* arg, char* pusrdata, int length), void (*disconn)(void* arg))
{
    struct https_context* context = arg;

    context->disconn = disconn;
    context->recv = recv;
}

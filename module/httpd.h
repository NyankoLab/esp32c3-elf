#pragma once

#include <esp_http_server.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HTTPD_FULL
#ifndef HTTPD_FULL

#define httpd_start _httpd_start
#define httpd_register_uri_handler _httpd_register_uri_handler
#define httpd_unregister_uri_handler _httpd_unregister_uri_handler
#define httpd_req_get_url_query_len _httpd_req_get_url_query_len
#define httpd_req_get_url_query_str _httpd_req_get_url_query_str
#define httpd_query_key_value _httpd_query_key_value
#define httpd_resp_send _httpd_resp_send
#define httpd_resp_send_chunk _httpd_resp_send_chunk
#define httpd_resp_set_status _httpd_resp_set_status
#define httpd_resp_set_type _httpd_resp_set_type
#define httpd_resp_set_hdr _httpd_resp_set_hdr

#define httpd_req_url_decode _httpd_req_url_decode
#define httpd_query_decode_key_value _httpd_query_decode_key_value

esp_err_t httpd_start(httpd_handle_t* handle, const httpd_config_t* config);
esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t* uri_handler);
esp_err_t httpd_unregister_uri_handler(httpd_handle_t handle, const char* uri, httpd_method_t method);
size_t httpd_req_get_url_query_len(httpd_req_t* r);
esp_err_t httpd_req_get_url_query_str(httpd_req_t* r, char* buf, size_t buf_len);
esp_err_t httpd_query_key_value(const char* qry, const char* key, char* val, size_t val_size);
esp_err_t httpd_resp_send(httpd_req_t* r, const char* buf, ssize_t buf_len);
esp_err_t httpd_resp_send_chunk(httpd_req_t* r, const char* buf, ssize_t buf_len);
esp_err_t httpd_resp_set_status(httpd_req_t* r, const char* status);
esp_err_t httpd_resp_set_type(httpd_req_t* r, const char* type);
esp_err_t httpd_resp_set_hdr(httpd_req_t* r, const char* field, const char* value);

#endif

char* httpd_req_url_decode(char* param);
char* httpd_query_decode_key_value(httpd_req_t* r, const char* key, char* val, size_t val_size);

#ifdef __cplusplus
}
#endif

#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <stdbool.h>
#include <curl/curl.h>

typedef struct {
    char *buffer;
    size_t size;
    bool is_valid;
} curl_buffer_t;

void destroy_curl_buffer(curl_buffer_t *buf);
size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *context);
CURL *make_handle(char *url, curl_buffer_t *cbuf);
char *replace_spaces_with_html_spaces(char *url);
curl_buffer_t get_page(char *url);

#endif // NETWORK_H

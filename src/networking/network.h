#ifndef NETWORK_H
#define NETWORK_H

#include <curl/curl.h>

typedef struct {
    char *buffer;
    size_t size;
} curl_buffer;

void destroy_curl_buffer(curl_buffer *buf);
size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *context);
CURL *make_handle(char *url, curl_buffer *cbuf);
char *replace_spaces_with_html_spaces(char *url);
curl_buffer *get_page(char *url);

#endif // NETWORK_H

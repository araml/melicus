#ifndef NETWORK_H
#define NETWORK_H

#include <curl/curl.h>

typedef struct {
    char *buffer;
    size_t size;
} curl_buffer;

size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *context);
CURL *make_handle(char *url, curl_buffer *cbuf);



#endif // NETWORK_H

#include <network.h>
#include <stdlib.h>

size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *context) {
    size_t size = sz * nmemb; // sz is the width of the byte, nmemb is the # of bytes
    curl_buffer *cbuf = (curl_buffer *) context;
    char *tmp = realloc(cbuf->buffer, cbuf->size + size);
    if (!tmp) {
        // Bad realloc

    }

    cbuf->buffer = tmp;
    for (size_t i = cbuf->size, k = 0; i < cbuf->size + size; i++, k++) {
        cbuf->buffer[i] = ((char *)contents)[k];
    }
    cbuf->size += size;

    return size;
}


CURL *make_handle(char *url, curl_buffer *cbuf) {
    CURL *handle = curl_easy_init();

    /* Important: use HTTP2 over HTTPS */
    curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(handle, CURLOPT_URL, url);

    /* buffer body */
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, grow_buffer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, cbuf);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, cbuf);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5L);
    return handle;
}



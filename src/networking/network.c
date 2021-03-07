#include <network.h>
#include <stdlib.h>
#include <string.h>
#include <string_utils.h>
#include <log.h>

void destroy_curl_buffer(curl_buffer *buf) {
    free(buf->buffer);
    free(buf);
}

size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *context) {
    size_t size = sz * nmemb; // sz is the width of the byte, nmemb is the # of bytes
    curl_buffer *cbuf = (curl_buffer *) context;
    char *tmp = (char *)realloc(cbuf->buffer, cbuf->size + size);
    if (!tmp) {
        // Bad realloc
    }

    memset(tmp + cbuf->size, 0, size);
    cbuf->buffer = tmp;
    for (size_t i = cbuf->size, k = 0; i < cbuf->size + size; i++, k++) {
        cbuf->buffer[i] = ((char *)contents)[k];
    }
    cbuf->size += size;

    return size;
}


CURL *make_handle(char *url, curl_buffer *cbuf) {
    CURL *handle = curl_easy_init();

    // Curl boilerplate.
    curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(handle, CURLOPT_URL, url);

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, grow_buffer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, cbuf);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, cbuf);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5L);

    // Follow location in case we get redirected because there is just a single
    // song with the name (at least in SM)
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    return handle;
}

char *replace_spaces_with_html_spaces(char *url) {
    size_t length_url = 0;
    for (size_t i = 0; i <= length(url); i++) {
        if (url[i] == ' ') {
            length_url += 3;
        } else {
            length_url++;
        }
    }

    LOG("String length: %zu\nNew length: %zu\n",
            length(url), length_url);

    char *tmp_url = (char *)malloc(length_url + 1);
    memset(tmp_url, 0, length_url + 1);

    for (size_t i = 0, k = 0; i <= length(url); i++) {
        if (url[i] == ' ') {
            tmp_url[k] = '%';
            tmp_url[k + 1] = '2';
            tmp_url[k + 2] = '2';
            k += 3;
        } else {
            tmp_url[k] = url[i];
            k++;
        }
    }

    return tmp_url;
}

curl_buffer *get_page(char *url) {
    curl_buffer *buf = (curl_buffer *)malloc(sizeof(curl_buffer));
    memset(buf, 0, sizeof(curl_buffer));
    
    size_t tries = 0;
    while (tries < 5) {
        CURL *handle = make_handle(url, buf);
        curl_easy_perform(handle);
        curl_easy_cleanup(handle);

        LOG("Get page ptr: %p sz: %zu", buf->buffer, buf->size);

        // error getting the page
        if (buf->buffer && buf->size != 0) {
            break;
        }
        tries++;
    }

    if (!buf->buffer || buf->size == 0) { 
        free(buf);
        return NULL;
    }

    char *tmp = (char *)realloc(buf->buffer, buf->size + 1);
    if (tmp) {
        buf->buffer = tmp;
        buf->size += 1;
    }
    buf->buffer[buf->size - 1] = '\0';
    return buf;
}

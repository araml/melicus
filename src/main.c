#include <ncurses.h>

#include <lyrics.h>
#include <cmus_status.h>
#include <string_utils.h>
#include <string.h>
#include <network.h>
#include <song_data.h>

song_data *current_song;

char *make_song_url(char *song_name) {
    if (!song_name)
        return NULL;

    // FIXME: add %20 not %
    size_t length_url = 0;
    size_t spaces = 0;
    for (size_t i = 0; i <= length(song_name); i++) {
        if (song_name[i] == ' ') {
            length_url += 3;
            spaces++;
        } else {
            length_url++;
        }
    }

    printf("String length: %zu\nNumber of spaces: %zu\nNew length: %zu\n",
            length(song_name), spaces, length_url);

    char *tmp_song_name = (char *)malloc(length_url + 1);
    memset(tmp_song_name, 0, length_url + 1);

    for (size_t i = 0, k = 0; i <= length(song_name); i++) {
        if (song_name[i] == ' ') {
            tmp_song_name[k] = '%';
            tmp_song_name[k + 1] = '2';
            tmp_song_name[k + 2] = '2';
            k += 3;
        } else {
            tmp_song_name[k] = song_name[i];
            k++;
        }
    }

    song_name = tmp_song_name;

    char prefix[] = "https://songmeanings.com/query/?query=";
    char postfix[] = "&type=songtitles";
    size_t url_length = length(prefix) + length(postfix) + length(song_name) + 1;
    char *url = (char *)malloc(url_length);
    memset(url, 0, url_length);
    memcpy(url, prefix, length(prefix));
    memcpy(url + length(prefix), song_name, length(song_name));
    memcpy(url + length(prefix) + length(song_name), postfix, length(postfix));
    printf("URL: %s\n", url);

    free(song_name);
    return url;
}

curl_buffer *get_page(char *url) {
    curl_buffer *buf = (curl_buffer *)malloc(sizeof(curl_buffer));
    memset(buf, 0, sizeof(curl_buffer));

    CURL *handle = make_handle(url, buf);
    curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    char *tmp = realloc(buf->buffer, buf->size + 1);
    if (tmp) {
        buf->buffer = tmp;
        buf->size += 1;
    }
    buf->buffer[buf->size - 1] = '\0';
    return buf;
}

char *find_link_for_song(char *page, song_data *s) {
    char *link = (char *)malloc(256); // No link (at least for SM) is longer than 256.
    memset(link, 0, 256);
    printf("Artist name: %s\n", s->artist_name);
    size_t idx = find_in_string(page, s->artist_name);
    if (idx == (size_t) - 1) // The song is not found
        return NULL;

    idx = idx - reverse_find(page + idx, s->song_name, idx);
    if (idx == (size_t) - 1)
        return NULL;

    char url_delim[] = "href=\"";
    idx = idx + length(url_delim) - reverse_find(page + idx, url_delim, idx);
    if (idx == (size_t) - 1)
        return NULL;

    for (size_t i = 0; i < 256 && page[idx + i] != '"'; i++) {
        link[i + 6] = page[idx + i];
    }
    //songmeanings.com/songs/view/3458764513820548010/

    link[0] = 'h';
    link[1] = link[2] = 't';
    link[3] = 'p';
    link[4] = 's';
    link[5] = ':';

    printf("Link: %s\n", link);

    return link;
}


char *get_lyrics(song_data *s) {
    // create song URL
    char *url = make_song_url(s->song_name);
    curl_buffer *buf = get_page(url);
    char *lyric = get_lyrics_from_page_string(buf->buffer);
    // If lyric is null then we didn't find the song so we have to find the link
    // to it in the HTML
    if (!lyric) {
        char *link = find_link_for_song(buf->buffer, s);
        destroy_curl_buffer(buf);
        buf = get_page(link);
        lyric = get_lyrics_from_page_string(buf->buffer);
        free(link);
    }

    destroy_curl_buffer(buf);
    free(url);
    return lyric;
}

int main(int argc, char *argv[]) {
    (void) argc; (void)argv;
    // Initialize current_song data
    current_song = NULL;

    // Get current song data
    song_data *s = get_current_song();

    if (s)
        printf("Artist: %s\nAlbum: %s\nSong: %s\n", s->artist_name, s->album, s->song_name);

    char *lyric = get_lyrics(s);
    printf("Lyric: \n%s\n", lyric);


    free(lyric);
    destroy_song_data(s);
    return 0;
}


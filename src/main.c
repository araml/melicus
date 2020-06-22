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
    return url;
}

int main(int argc, char *argv[]) {
    (void) argc; (void)argv;
    // Initialize current_song data
    current_song = NULL;

    // Get current song data
    song_data *s = get_current_song();
    if (s)
        printf("Artist: %s\nAlbum: %s\nSong: %s\n", s->name, s->album, s->song_name);

    // create song URL
    char *url = make_song_url(s->song_name);

    curl_buffer *buf = (curl_buffer *)malloc(sizeof(curl_buffer));
    memset(buf, 0, sizeof(curl_buffer));
   // buf->buffer = NULL;
   // buf->size = 0;


    CURL *handle = make_handle(url, buf);
    curl_easy_perform(handle);
    //printf("%s", buf->buffer);
    curl_easy_cleanup(handle);

    char *lyric = get_lyrics_from_page_string(buf->buffer);
    printf("Lyric: \n%s\n", lyric);

    free(lyric);
    free(buf->buffer);
    free(buf);
    destroy_song_data(s);
    free(url);
    return 0;
}


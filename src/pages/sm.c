#include <sm.h>
#include <stdlib.h>
#include <string_utils.h>
#include <log.h>
#include <network.h>
#include <string.h>

char *sm_make_song_url(song_data *data) {
    if (!data->song_name)
        return NULL;

    char *song_name = replace_spaces_with_html_spaces(data->song_name);
    //TODO: add musixmatch backend
    //https://www.musixmatch.com/lyrics/Band-Name/Song-Name


    // TODO: refactor this shitty string concat lol
    char prefix[] = "https://songmeanings.com/query/?query=%20";
    char space[] = "%20";
    char postfix[] = "&type=songtitles";

    char *url = NULL;
    add_to_string(&url, prefix);
    add_to_string(&url, song_name);
    add_to_string(&url, space);
    if (data->artist_name) {
        char *artist_name = replace_spaces_with_html_spaces(data->artist_name);
        add_to_string(&url, artist_name);
        free(artist_name);
    }
    add_to_string(&url, postfix);

    LOG("URL: %s\n", url);

    free(song_name);
    return url;
}

#define ERR_FIND ((size_t) - 1)

char *sm_find_link_for_song(char *page, song_data *s) {
    char song_table[] = "songs table";
    char title[] = "title";
    char song_href[] = "href=\"";

    size_t idx = 0;
    size_t total_idx = 0;

    while (true) {

        idx = find_in_string(page, song_table);
        total_idx = idx;
        if (idx == ERR_FIND) {
            LOG("Error no song table");
            return NULL;
        }

        page += idx;
        total_idx += idx;

        idx = find_in_string(page, s->song_name);
        if (idx == ERR_FIND) {
            LOG("Error no song name in song table");
            return NULL;
        }

        page += idx;
        total_idx += idx;

        idx = find_in_string(page, title);
        /* Asume theres always a title= if we found a song name, so don't check for
         * errors
         */


        size_t artist_idx = find_in_string(page, s->artist_name);

        /* This checks that the distance to the artist name is closer to this
         * song than to the next one.
         * horrivel 8{
         */
        if (artist_idx > 0 && artist_idx - idx < 50) {
            /* We have found a match for both song and title */
            size_t less = reverse_find(page, song_href, total_idx);
            page -= less;
            page += sizeof(song_href) - 1;
            //page += 7; // title="
            char *link = (char *)malloc(128);
            memset(link, 0, 128);

            for (size_t k = 0; page[k] != '"'; k++) {
                link[k + 8] = page[k];
            }

            link[0] = 'h';
            link[1] = link[2] = 't';
            link[3] = 'p';
            link[4] = 's';
            link[5] = ':';
            link[6] = link[7] = '/';
            LOG("New url %s\n", link);
            return link;
        } else {
            page += artist_idx + idx;
        }
    }

    return NULL;
}

char *sm_get_lyrics_from_page_string(const char *page_string) {
    /* We don't know if we got redirected to a page with the song or not
     * we'll try to match with lyrics first
     * if we can't find any then we'll search for links to any lyrics
     * if then we can't find any then the song isn't in the database.
     */


    // Tries to match the lyrics inside the html
    char opening_div[] = "<div class=\"holder lyric-box\">";
    size_t idx = find_in_string(page_string, opening_div);
    if (idx == (size_t)-1)
        return NULL;

    idx += length(opening_div);

    char *lyrics = (char *)calloc(1, 1);
    size_t k = 0;
    size_t lyrics_length = 1;
    size_t page_string_length = length(page_string);
    for (size_t i = idx; i < page_string_length; i++) {
        if (k >= lyrics_length) {
            char *tmp = (char *)realloc(lyrics, lyrics_length *= 2);
            if (tmp)
                lyrics = tmp;
        }

        // very ugly
        // basically breaks when it finds the closing <div
        if (page_string[i] == '<' && page_string[i + 1] == 'd')
            break;
        lyrics[k++] = page_string[i];
    }
    lyrics[k] = '\0';
    return lyrics;
}

string_split *sm_clean_lyrics(char *lyrics) {
    string_split *sv = create_string_split();
    char *line = NULL;
    for (size_t i = 0; i < length(lyrics); i++) {
        // </br>
        if (lyrics[i] == '\t' || lyrics[i] == '\n' || lyrics[i] == '\r')
            continue;
        if (lyrics[i]     == '<' && lyrics[i + 1] == 'b' &&
            lyrics[i + 2] == 'r' && lyrics[i + 3] == '/' && lyrics[i + 4] == '>') {
            i += 4;
            line ? push_to_string_split(sv, line) : push_to_string_split(sv, "\n");
            free(line);
            line = NULL;
            continue;
        }

        add_char_to_string(&line, lyrics[i]);
    }
    return sv;
}

string_split *sm_get_lyrics(song_data *s) {
    // create song URL
    char *url = sm_make_song_url(s);
    curl_buffer *buf = get_page(url);
    char *lyric = sm_get_lyrics_from_page_string(buf->buffer);
    // If lyric is null then we didn't find the song so we have to find the link
    // to it in the HTML
    if (!lyric) {
        char *link = sm_find_link_for_song(buf->buffer, s);
        free(lyric);
        destroy_curl_buffer(buf);
        buf = get_page(link);
        lyric = sm_get_lyrics_from_page_string(buf->buffer);
        free(link);
    }

    string_split *lyrics = NULL;
    if (lyric) {
        lyrics = sm_clean_lyrics(lyric);
    }

    free(lyric);
    destroy_curl_buffer(buf);
    free(url);
    return lyrics;
}



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

char *sm_find_link_for_song(char *page, song_data *s) {
    //LOG("%s", page);
    char prefix_find_artist[] = "title=\"";
    char jjj = '"';
    char *find_artist_name = (char *)malloc(length(prefix_find_artist) +
                                            length(s->artist_name) + 2);
    memset(find_artist_name, 0, length(prefix_find_artist) + length(s->artist_name) + 2);
    memcpy(find_artist_name, prefix_find_artist, length(prefix_find_artist));
    memcpy(find_artist_name + length(prefix_find_artist), s->artist_name, length(s->artist_name));
    memcpy(find_artist_name + length(prefix_find_artist) + length(s->artist_name), &jjj, 1);

    LOG("Find artist name: %s\n", find_artist_name);

    size_t idx = find_in_string(page, find_artist_name);
    if (idx == (size_t) - 1) { // The song is not found
        return NULL;
    }
    LOG("Index is %zu %zu\n", idx, -1);

    // FIXME: This == -1 check is wrong since reverse find will return 0
    // when it doesn't finds the string
    idx = idx - reverse_find(page + idx, s->song_name, idx);
    if (idx == (size_t) - 1)
        return NULL;

    LOG("Index is %zu %zu\n", idx, -1);

    char url_delim[] = "href=\"";
    idx = idx + length(url_delim) - reverse_find(page + idx, url_delim, idx);
    // FIXME: same as the fixme above (this is *almost* never == -1, needs
    // a ttmp idx to compare, and reverse_find should return -1 in that case
    if (idx == (size_t) - 1)
        return NULL;

    char *link = (char *)malloc(256); // No link (at least for SM) is longer than 256.
    memset(link, 0, 256);

    for (size_t i = 0; i < 256 && page[idx + i] != '"'; i++) {
        link[i + 6] = page[idx + i];
    }
    //songmeanings.com/songs/view/3458764513820548010/

    link[0] = 'h';
    link[1] = link[2] = 't';
    link[3] = 'p';
    link[4] = 's';
    link[5] = ':';

    LOG("Link %s\n", link);
    return link;
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
    string_split *sv = create_string_ssplit();
    char *line = NULL;
    for (size_t i = 0; i < length(lyrics); i++) {
        // </br>
        if (lyrics[i] == '\t' || lyrics[i] == '\n' || lyrics[i] == '\r')
            continue;
        if (lyrics[i]     == '<' && lyrics[i + 1] == 'b' &&
            lyrics[i + 2] == 'r' && lyrics[i + 3] == '/' && lyrics[i + 4] == '>') {
            i += 4;
            push_to_string_split(sv, line);
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



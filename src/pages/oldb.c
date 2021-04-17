#include <oldb.h>
#include <stdlib.h>
#include <string.h>
#include <network.h>
#include <string_utils.h>


string_split *oldb_clean_lyrics(char *lyrics) {
    string_split *sv = create_string_split();
    const char opening_div[] = "<div id=\"lyrics\">";
    const char ending_string[] = "Song name";
    size_t idx = find_in_string(lyrics, opening_div);
    idx += length(opening_div);
    size_t ending = find_in_string(lyrics, ending_string);

    char *line = NULL;

    const char br[] = "<br/>";

    for (size_t i = idx; i < ending; i++) {
        if (lyrics[i] == '<' && match(lyrics + i, br, ending - i)) {
            i += 4;
            line ? push_to_string_split(sv, line) : push_to_string_split(sv, "\n");
            free(line);
            line = NULL;
            continue;
        } else if (lyrics[i] == '<') { // skip any other html tag 
            for (; i < ending; i++)
                if (lyrics[i] == '>')
                    break;
            continue; // awful
        }

        add_char_to_string(&line, lyrics[i]);
    }
    return sv;
}

char* make_oldb_url(const song_data *s) { 
    const char prefix[] = "https://lyrics.github.io/db/";

    char *url = NULL;
    add_to_string(&url, prefix);

    // url schema prefix/db/X/Artist Name/Album Name/Song Name
    // All strings should be Upper Cased
    char first_letter = uppercase_char(s->artist_name[0]);
    // open lyrics db doesn't support non-ascii artists :/
    if (!first_letter) 
        return NULL;
    add_char_to_string(&url, first_letter);
    
    char *uppercased_artist = uppercase_string(s->artist_name);
    char *uppercased_album = uppercase_string(s->album);
    char *uppercased_song_name = uppercase_string(s->song_name);
    if (!uppercased_artist || !uppercased_album || !uppercased_song_name) { 
        free(uppercased_artist);
        free(uppercased_album);
        free(uppercased_song_name);
        return NULL;
    }

    char *h_artist = replace_spaces_with_html_spaces(uppercased_artist);
    char *h_album = replace_spaces_with_html_spaces(uppercased_album);
    char *h_song_name = replace_spaces_with_html_spaces(uppercased_song_name);

    add_char_to_string(&url, '/');
    add_to_string(&url, h_artist);
    add_char_to_string(&url, '/');
    add_to_string(&url, h_album);
    add_char_to_string(&url, '/');
    add_to_string(&url, h_song_name);
    add_char_to_string(&url, '/');

    free(h_album);
    free(h_artist);
    free(h_song_name);
    free(uppercased_artist);
    free(uppercased_song_name);
    free(uppercased_album);

    return url;
}

string_split *oldb_get_lyrics(const song_data *s) {
    if (!s || !s->artist_name || !s->album || !s->song_name) 
        return NULL;
        
    char *url = make_oldb_url(s);
    curl_buffer *buf = get_page(url);
    free(url);

    string_split *lyrics = oldb_clean_lyrics(buf->buffer);
    destroy_curl_buffer(buf);
    return lyrics;
}

#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <ncurses.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <locale.h>
#include <langinfo.h>

#include <log.h>
#include <lyrics.h>
#include <network.h>
#include <song_data.h>
#include <cmus_status.h>
#include <string_utils.h>


song_data *current_song;
string_split *current_song_lyrics;

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

    // TODO: do this for the artist name
    // TODO 2: refactor this into its own function in networking
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

char *make_song_url(song_data *data) {
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

curl_buffer *get_page(char *url) {
    curl_buffer *buf = (curl_buffer *)malloc(sizeof(curl_buffer));
    memset(buf, 0, sizeof(curl_buffer));

    CURL *handle = make_handle(url, buf);
    curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    char *tmp = (char *)realloc(buf->buffer, buf->size + 1);
    if (tmp) {
        buf->buffer = tmp;
        buf->size += 1;
    }
    buf->buffer[buf->size - 1] = '\0';
    return buf;
}

char *find_link_for_song(char *page, song_data *s) {
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


char *get_lyrics(song_data *s) {
    // create song URL
    char *url = make_song_url(s);
    curl_buffer *buf = get_page(url);
    char *lyric = get_lyrics_from_page_string(buf->buffer);
    // If lyric is null then we didn't find the song so we have to find the link
    // to it in the HTML
    if (!lyric) {
        char *link = find_link_for_song(buf->buffer, s);
        free(lyric);
        destroy_curl_buffer(buf);
        buf = get_page(link);
        lyric = get_lyrics_from_page_string(buf->buffer);
        free(link);
    }

    destroy_curl_buffer(buf);
    free(url);
    return lyric;
}

string_split* create_string_ssplit() {
    string_split *r = (string_split *)malloc(sizeof(string_split));
    r->strings = NULL;
    r->size = 0;
    r->reserved_size = 0;
    return r;
}

int push_to_string_split(string_split *sv, char *line) {
    if (sv->reserved_size == 0) {
        sv->strings = (char **)malloc(sizeof(char *));
        sv->reserved_size = 1;
    } else if (sv->size == sv->reserved_size) {
        char **tmp = (char **)realloc(sv->strings, sizeof(char *) *
                                      (sv->reserved_size *= 2));
        if (tmp) {
            sv->strings = tmp;
        } else {
            return -1;
        }
    }

    sv->strings[sv->size++] = line;
    return 0;
}

string_split *clean_lyrics(char *lyrics) {
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

string_split *no_lyrics() {
    return NULL;
}

bool new_song(song_data *old_song, song_data *new_song) {
    // Asume no two album names are equal
    bool song_eq = string_cmp(old_song->song_name, new_song->song_name);
    bool artist_eq = string_cmp(old_song->artist_name, new_song->artist_name);
    return (song_eq && !artist_eq) || !song_eq;

}

size_t height, width;

struct winsize get_term_size() {
    struct winsize size;
    ioctl(1, TIOCGWINSZ, &size);
    return size;
}

void init_window() {

}



bool refresh_screen = false;

void draw_screen() {
    size_t idx = 0;
    wclear(stdscr);
    mvprintw(0, center_text(codepoints(current_song->artist_name), width),
            "%s", current_song->artist_name);
    mvprintw(1, center_text(codepoints(current_song->album), width),
            "%s", current_song->album);
    mvprintw(2, center_text(codepoints(current_song->song_name), width),
            "%s", current_song->song_name);
    for (size_t i = 4, k = 0; i < height - 1 && k <
            current_song_lyrics->size; i++, k++) {
        if (current_song_lyrics->strings[k + idx]) {
            size_t pos = center_text(codepoints(current_song_lyrics->strings[k + idx]), width);
            LOG("Pos: %zu\n", pos);
            LOG("Length: %zu\n", codepoints(current_song_lyrics->strings[k + idx]));
            LOG("%s\n", current_song_lyrics->strings[k + idx]);
            mvaddstr(i, pos, current_song_lyrics->strings[k + idx]);
        } else {
            LOG("Newline \n\n");
        }
    }
    refresh_screen = false;
    wrefresh(stdscr);

}

bool window_size_changed = false;

void resize_window() {
    window_size_changed = false;
    struct winsize size = get_term_size();
    resizeterm(size.ws_row, size.ws_col);
    height = size.ws_row;
    width = size.ws_col;
    draw_screen();
}

void sig_winch(__attribute__((unused)) int irq) {
    window_size_changed = true;
}

int main(int argc, char *argv[]) {
    (void) argc; (void)argv;
    // Initialize current_song data
    current_song = NULL;
    current_song_lyrics = NULL;
    //string_split *l = NULL;

    struct winsize max;
    ioctl(1, TIOCGWINSZ, &max);
    LOG("Screen size %d %d\n", max.ws_row, max.ws_col);
    height = max.ws_row;
    width = max.ws_col;
    setlocale(LC_ALL, "");
    initscr();
    nodelay(stdscr, true);
    curs_set(0);
    noecho();
    signal(SIGWINCH, sig_winch);

    while (true) {
        song_data *s = get_current_song();
        if (!s) {
            continue;
        }

        if (!current_song || new_song(current_song, s)) {
            current_song = s;
            char *lyrics = get_lyrics(s);

            LOG("Old song: %s\n", current_song->song_name);

            if (current_song_lyrics)
                destroy_string_split(current_song_lyrics);
            current_song_lyrics = clean_lyrics(lyrics);
            LOG("New song: %s\n", current_song->song_name);
            refresh_screen = true;
            free(lyrics);
        } else {
            sched_yield();
        }

        if (current_song_lyrics && refresh_screen) {
            draw_screen();
        }

        if (window_size_changed) {
            resize_window();
        }

        if (getch() == 'q') {
            break;
        }

        usleep(10000);
    }

    endwin();
    destroy_string_split(current_song_lyrics);
    destroy_song_data(current_song);
    return 0;
}


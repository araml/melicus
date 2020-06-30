#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <ncurses.h>

#include <lyrics.h>
#include <cmus_status.h>
#include <string_utils.h>

#include <string.h>
#include <network.h>
#include <song_data.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdio.h>

song_data *current_song;
string_split *current_song_lyrics;

void log_melicus(const char *format, ...) {
    va_list arg;
    int fd = open("melicus.log", O_CREAT | O_APPEND | O_WRONLY,
                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                 S_IROTH | S_IWOTH);
    va_start(arg, format);
    vdprintf(fd, format, arg);
    va_end(arg);
}

// TODO: Proper log function
#define LOG(...) log_melicus(__VA_ARGS__)

char *make_song_url(song_data *data) {
    char *song_name = data->song_name;
    char *artist_name = data->artist_name;
    if (!song_name)
        return NULL;

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

    LOG("String length: %zu\nNumber of spaces: %zu\nNew length: %zu\n",
            length(song_name), spaces, length_url);

    char *tmp_song_name = (char *)malloc(length_url + 1);
    memset(tmp_song_name, 0, length_url + 1);

    // TODO: do this for the artist name
    // TODO 2: refactor this into its own function in networking
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
    //TODO: add musixmatch backend
    //https://www.musixmatch.com/lyrics/Band-Name/Song-Name


    // TODO: refactor this shitty string concat lol
    char prefix[] = "https://songmeanings.com/query/?query=%20";
    char space[] = "%20";
    char postfix[] = "&type=songtitles";
    size_t url_length = length(prefix) + length(postfix) + length(song_name) +
                        length(artist_name) + length(space) + 1;
    char *url = (char *)malloc(url_length);
    memset(url, 0, url_length);
    memcpy(url, prefix, length(prefix));
    memcpy(url + length(prefix), song_name, length(song_name));
    memcpy(url + length(prefix) + length(song_name), space, length(space));
    memcpy(url + length(prefix) + length(song_name) + length(space),
           artist_name, length(artist_name));
    memcpy(url + length(prefix) + length(song_name) + length(space) +
           length(artist_name), postfix, length(postfix));
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

    char *tmp = realloc(buf->buffer, buf->size + 1);
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

    idx = idx - reverse_find(page + idx, s->song_name, idx);
    if (idx == (size_t) - 1)
        return NULL;

    char url_delim[] = "href=\"";
    idx = idx + length(url_delim) - reverse_find(page + idx, url_delim, idx);
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

int add_to_string(char **s, char c) {
    if (*s == NULL) {
        *s = (char *)malloc(2);
        (*s)[0] = c;
        (*s)[1] = '\0';
    } else {
        char *tmp = realloc(*s, length(*s) + 2);
        if (tmp)
            *s = tmp;
        else
            return -1;
        size_t s_length = length(*s);
        (*s)[s_length] = c;
        (*s)[s_length + 1] = '\0';
    }

    return 0;
}

string_split* create_string_ssplit() {
    string_split *r = malloc(sizeof(string_split));
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

        add_to_string(&line, lyrics[i]);
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

int main(int argc, char *argv[]) {
    (void) argc; (void)argv;
    // Initialize current_song data
    current_song = NULL;
    current_song_lyrics = NULL;
    //string_split *l = NULL;

    struct winsize max;
    ioctl(1, TIOCGWINSZ, &max);
    LOG("Screen size %d %d\n", max.ws_row, max.ws_col);
    initscr();
    nodelay(stdscr, true);
    curs_set(0);
    noecho();

    bool refresh_screen = false;
    size_t idx = 0;
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
            wclear(stdscr);
            mvprintw(0, center_text(length(current_song->artist_name), max.ws_col),
                    "%s", current_song->artist_name);
            mvprintw(1, center_text(length(current_song->album), max.ws_col),
                    "%s", current_song->album);
            mvprintw(2, center_text(length(current_song->song_name), max.ws_col),
                    "%s", current_song->song_name);
            for (size_t i = 4, k = 0; i < max.ws_row - 1 && k <
                    current_song_lyrics->size; i++, k++) {
                if (current_song_lyrics->strings[k + idx]) {
                    size_t pos = center_text(length(current_song_lyrics->strings[k + idx]), max.ws_col);
                    LOG("Pos: %zu\n", pos);
                    LOG("Length: %zu\n", length(current_song_lyrics->strings[k + idx]));
                    LOG("%s\n", current_song_lyrics->strings[k + idx]);
                    mvprintw(i, pos, "%s", current_song_lyrics->strings[k + idx]);
                }
            }
            refresh_screen = false;
            wrefresh(stdscr);
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


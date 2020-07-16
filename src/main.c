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

#include <sm.h>
#include <log.h>
#include <lyrics.h>
#include <network.h>
#include <song_data.h>
#include <cmus_status.h>
#include <string_utils.h>

song_data *current_song;
string_split *current_song_lyrics;

string_split *(* get_lyrics)(song_data *) = sm_get_lyrics;

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

WINDOW *lyrics_pad = NULL;
size_t pad_height, pad_width;
WINDOW *title_pad = NULL;

void init_window() {
    //struct winsize size = get_term_size();
    //main_pad = newpad(
}

void create_pad(WINDOW **w, int height) {
    if (*w) {
        delwin(*w);
    }

    *w = newpad(height, 80);
    nodelay(*w, true);
    LOG("Pad created with: %zu %d\n", height, 80);
}

bool refresh_screen = false;

int min(int a, int b) {
    return a > b ? b : a;
}

int center_position(char *text) {
    int _width = min(80, width);
    return center_text(codepoints(text), _width);
}

#define LYRICS_WIDTH 80

int next_word_size(char *line) {
    if (!line)
        return 0;
    int sz = 0, i = 0;
    while (true) {
        if (line[i] == '\0' || line[i] == ' ') {
            break;
        }
        i++;
        sz++;
    }

    return sz;
}

string_split *split_line(char *line) {
    string_split *s = create_string_split();
    char *word = NULL;
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == ' ') {
            push_to_string_split(s, word);
            free(word);
            word = NULL;
        } else {
            add_char_to_string(&word, line[i]);
        }
    }

    if (word) {
        push_to_string_split(s, word);
        free(word);
    }

    return s;
}

int count_used_rows(char *line, __attribute__((unused)) int lyrics_width) {
    if (!line)
        return 1;

    int rows = 0;
    int idx = 0;

    string_split *s = split_line(line);
    while (true) {
        if (idx == s->size)
            break;

        size_t l = length(s->strings[idx]);
        idx++;
        // If the word is too big then its going to fill its own lines
        if (l >= 80) {
            rows += (l / 80) + (l % 80 ? 1 : 0);
            continue;
        }

        l++;
        /* If the word is not big enough then we can add it to the current line
         * and try tp keep adding more words to it
         */
        while (true) {
            if (idx == s->size) {
                rows++;
                break;
            }
            // + 1 for the whitespace between words
            if (l + length(s->strings[idx]) + 1 < 80) {
                l = l + length(s->strings[idx]) + 1;
                idx++;
            } else {
                rows++;
                break;
            }
        }
    }

    destroy_string_split(s);

    return rows;
}

int calculate_lyrics_height(string_split *lyrics) {
    int total_rows = 0;
    for (int i = 0; i < lyrics->size; i++) {
        if (!lyrics->strings[i]) {
            total_rows++;
            continue;
        } else {
            total_rows += count_used_rows(lyrics->strings[i], LYRICS_WIDTH);
        }
    }

    return total_rows;
}

typedef struct {
    int next_idx;
    int nrows;
} xx;

xx get_print_pos(string_split *s, __attribute__((unused)) int lyrics_width,
                     int idx) {
    int rows = 0;
    xx ps;

    while (true) {
        if (idx == s->size)
            break;

        size_t l = length(s->strings[idx]);
        idx++;
        // If the word is too big then its going to fill its own lines
        if (l >= 80) {
            rows += (l / 80) + (l % 80 ? 1 : 0);
            goto end;
        }

        l++;
        /* If the word is not big enough then we can add it to the current line
         * and try tp keep adding more words to it
         */
        while (true) {
            if (idx == s->size) {
                rows++;
                goto end;
            }
            // + 1 for the whitespace between words
            if (l + length(s->strings[idx]) + 1 < 80) {
                l = l + length(s->strings[idx]) + 1;
                idx++;
            } else {
                rows++;
                goto end;
            }
        }
    }

end:
    ps.nrows = rows;
    ps.next_idx = idx;
    return ps;

}

void draw_lyrics() {
    for (size_t k = 0, i = 0; k < current_song_lyrics->size; k++) {
        if (current_song_lyrics->strings[k]) {
            int idx = 0;
            string_split *l = split_line(current_song_lyrics->strings[k]);
            while (idx != l->size) {
                xx rs = get_print_pos(l, 80, idx);
                if (rs.nrows > 1) {
                    size_t pos = center_position(current_song_lyrics->strings[k]);
                    mvwaddstr(lyrics_pad, i, pos, l->strings[idx]);
                } else {
                    char buf[81];
                    memset(buf, 0, 80);
                    int buf_idx = 0;
                    for (int p = idx; p < rs.next_idx; p++) {
                        memcpy(buf + buf_idx, l->strings[p], length(l->strings[p]));
                        buf_idx += length(l->strings[p]);
                        if (buf_idx != 80) {
                            buf[buf_idx] = ' ';
                            buf_idx++;
                        }
                    }

                    size_t pos = center_position(buf);
                    mvwaddstr(lyrics_pad, i, pos, buf);
                }
                idx = rs.next_idx;
                i += rs.nrows;
            }

            destroy_string_split(l);

            //i++;
        } else {
            LOG("Newline \n\n");
            i++;
        }
    }
}


void draw_screen() {
    create_pad(&title_pad, 3);
    int lyrics_height = calculate_lyrics_height(current_song_lyrics);
    create_pad(&lyrics_pad, lyrics_height);
    LOG("lyric height calc %d\n", calculate_lyrics_height(current_song_lyrics));

    wclear(stdscr);
    mvwaddstr(title_pad, 0, center_position(current_song->artist_name),
              current_song->artist_name);
    mvwaddstr(title_pad, 1, center_position(current_song->album), current_song->album);
    mvwaddstr(title_pad, 2, center_position(current_song->song_name), current_song->song_name);

    draw_lyrics();

    /*           (of pad        )     (rectangle on the screen)
     * prefresh(pad, y start, x start, y, x, h, w)
     *
     */

    int pad_position = center_text(80, width);

    refresh();
    prefresh(title_pad, 0, 0, 0, pad_position, 3, width -2);
    prefresh(lyrics_pad, 0, 0, 4, pad_position, height - 2, width - 2);
    refresh_screen = false;
    //refresh();
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
    cbreak();
    signal(SIGWINCH, sig_winch);

    while (true) {
        song_data *s = get_current_song();
        if (!s) {
            continue;
        }

        if (!current_song || new_song(current_song, s)) {
            if (current_song) {
                destroy_song_data(current_song);
            }

            current_song = s;

            string_split *new_lyrics = get_lyrics(s);

            LOG("Old song: %s\n", current_song->song_name);

            if (current_song_lyrics)
                destroy_string_split(current_song_lyrics);
            current_song_lyrics = new_lyrics;
            LOG("New song: %s\n", current_song->song_name);
            refresh_screen = true;
        } else {
            destroy_song_data(s);
            sched_yield();
        }

        if (getch() == 'q') {
            break;
        }

        if (current_song_lyrics && refresh_screen) {
            draw_screen();
        }

        if (window_size_changed) {
            resize_window();
        }

        wrefresh(title_pad);
        wrefresh(lyrics_pad);
    }

    endwin();
    destroy_song_data(current_song);
    destroy_string_split(current_song_lyrics);
    return 0;
}


#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <curses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#include <sm.h>
#include <oldb.h>
#include <log.h>
#include <window.h>
#include <lyrics.h>
#include <song_data.h>
#include <cmus_status.h>
#include <string_utils.h>

song_data_t current_song;
string_split_t current_song_lyrics;
string_split_t current_song_lyrics_fixed_for_width;

string_split_t (* get_lyrics)(const song_data_t *) = oldb_get_lyrics;//sm_get_lyrics;

string_split_t no_lyrics() {
    return create_invalid_string_split();
}

bool new_song(song_data_t *old_song, song_data_t *new_song) {
    // Asume no two album names are equal
    bool song_eq = string_cmp(old_song->song_name, new_song->song_name);
    bool artist_eq = string_cmp(old_song->artist_name, new_song->artist_name);
    return (song_eq && !artist_eq) || !song_eq;

}

WINDOW *lyrics_pad = NULL;
size_t pad_height, pad_width;
WINDOW *title_pad = NULL;

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

string_split_t split_line(char *line) {
    string_split_t s = create_string_split();
    char *word = NULL;
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == ' ') {
            push_to_string_split(&s, word);
            free(word);
            word = NULL;
        } else {
            add_char_to_string(&word, line[i]);
        }
    }

    if (word) {
        push_to_string_split(&s, word);
        free(word);
    }

    return s;
}

typedef struct {
    int next_idx;
    int nrows;
} xx;

/*
 * Gets number of rows occupied by a line (divides line by number of chars in a
 * row)
 * it also returns the index to the next element in this long line for when its
 * spliting lines
 */

xx get_print_pos(string_split_t *s, __attribute__((unused)) int lyrics_width,
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

void resize_lyrics() {
    if (!current_song_lyrics.is_valid)
        return;
    destroy_string_split(&current_song_lyrics_fixed_for_width);
    current_song_lyrics_fixed_for_width = create_string_split();

    for (size_t k = 0, i = 0; k < current_song_lyrics.size; k++) {
        if (current_song_lyrics.strings[k]) {
            int idx = 0;
            string_split_t l = split_line(current_song_lyrics.strings[k]);
            while (idx != l.size) {
                xx rs = get_print_pos(&l, 80, idx);
                if (rs.nrows > 1) {
                    push_to_string_split(&current_song_lyrics_fixed_for_width, l.strings[idx]);
                } else {
                    char buf[81];
                    memset(buf, 0, 80);
                    int buf_idx = 0;
                    for (int p = idx; p < rs.next_idx; p++) {
                        memcpy(buf + buf_idx, l.strings[p], length(l.strings[p]));
                        buf_idx += length(l.strings[p]);
                        if (buf_idx != 80) {
                            buf[buf_idx] = ' ';
                            buf_idx++;
                        }
                    }

                    push_to_string_split(&current_song_lyrics_fixed_for_width, buf);
                }
                idx = rs.next_idx;
                i += rs.nrows;
            }

            destroy_string_split(&l);
        } else {
            LOG("Newline \n\n");
            i++;
        }
    }
}

void draw_lyrics() {
    if (!current_song_lyrics.is_valid)
        return; 
    for (size_t i = 0; i < current_song_lyrics_fixed_for_width.size; i++) {
        size_t pos = center_position(current_song_lyrics_fixed_for_width.strings[i]);
        mvwaddstr(lyrics_pad, i, pos, current_song_lyrics_fixed_for_width.strings[i]);
    }
}



void draw_screen() {
    create_pad(&title_pad, 3);
    if (current_song_lyrics_fixed_for_width.is_valid) { 
        int lyrics_height = current_song_lyrics_fixed_for_width.size;
        create_pad(&lyrics_pad, lyrics_height);
        LOG("lyric height calc %d\n", current_song_lyrics_fixed_for_width.size);
    }
    wclear(stdscr);
    mvwaddstr(title_pad, 0, center_position(current_song.artist_name),
              current_song.artist_name);
    mvwaddstr(title_pad, 1, center_position(current_song.album), current_song.album);
    mvwaddstr(title_pad, 2, center_position(current_song.song_name), current_song.song_name);

    draw_lyrics();
    draw_status_bar();


    /*           (of pad        )     (rectangle on the screen)
     * prefresh(pad, y start, x start, y, x, h, w)
     *
     */

    int pad_position = center_text(80, width);

    refresh();
    prefresh(title_pad, 0, 0, 0, pad_position, 3, width -2);
    prefresh(lyrics_pad, 0, 0, 4, pad_position, height - 4, width - 2);
    refresh_screen = false;
    //refresh();
}

void resize_window() {
    window_size_changed = false;
    struct winsize size = get_term_size();
    resizeterm(size.ws_row, size.ws_col);
    height = size.ws_row;
    width = size.ws_col;
    resize_lyrics();
    draw_screen();
}

int main(int argc, char *argv[]) {
    (void) argc; (void)argv;
    // Initialize current_song data
    current_song = create_invalid_song_data();
    current_song_lyrics = create_invalid_string_split();
    //string_split *l = NULL;
    log_init();
    window_init();

    while (true) {
        song_data_t s = get_current_song();
        if (!s.is_valid) {
            continue;
        }

        if (!current_song.is_valid || new_song(&current_song, &s)) {
            if (current_song.is_valid) {
                destroy_song_data(&current_song);
            }

            current_song = s;
            
            // FIXME(aram): if we can't find lyrics show some kind of error message
            string_split_t new_lyrics = get_lyrics(&s);

            LOG("Old song: %s\n", current_song.song_name);

            if (current_song_lyrics.is_valid)
                destroy_string_split(&current_song_lyrics);
            current_song_lyrics = new_lyrics;
            LOG("New song: %s\n", current_song.song_name);
            resize_lyrics();
            refresh_screen = true;
        } else {
            destroy_song_data(&s);
            sched_yield();
        }

        if (getch() == 'q') {
            break;
        }

        if (refresh_screen) {
            draw_screen();
        }

        if (window_size_changed) {
            resize_window();
        }

        wrefresh(title_pad);
        wrefresh(lyrics_pad);
    }

    endwin();
    destroy_song_data(&current_song);
    destroy_string_split(&current_song_lyrics);
    return 0;
}


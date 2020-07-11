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

void draw_screen() {
    create_pad(&title_pad, 3);
    create_pad(&lyrics_pad, current_song_lyrics->size * 2);
    size_t idx = 0;
    wclear(stdscr);
    mvwaddstr(title_pad, 0, center_position(current_song->artist_name),
              current_song->artist_name);
    mvwaddstr(title_pad, 1, center_position(current_song->album), current_song->album);
    mvwaddstr(title_pad, 2, center_position(current_song->song_name), current_song->song_name);
    for (size_t i = 0, k = 0; i < height - 1 && k <
            current_song_lyrics->size; i++, k++) {
        if (current_song_lyrics->strings[k + idx]) {
            size_t pos = center_position(current_song_lyrics->strings[k + idx]);
            LOG("Pos: %zu\n", pos);
            LOG("Length: %zu\n", codepoints(current_song_lyrics->strings[k + idx]));
            LOG("%s\n", current_song_lyrics->strings[k + idx]);
            mvwaddstr(lyrics_pad, i, pos, current_song_lyrics->strings[k + idx]);
        } else {
            LOG("Newline \n\n");
        }
    }

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
            current_song = s;
            string_split *new_lyrics = get_lyrics(s);

            LOG("Old song: %s\n", current_song->song_name);

            if (current_song_lyrics)
                destroy_string_split(current_song_lyrics);
            current_song_lyrics = new_lyrics;
            LOG("New song: %s\n", current_song->song_name);
            refresh_screen = true;
        } else {
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
    destroy_string_split(current_song_lyrics);
    destroy_song_data(current_song);
    return 0;
}


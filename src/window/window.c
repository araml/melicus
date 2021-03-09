#include <log.h>
#include <locale.h>
#include <signal.h>
#include <curses.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/ioctl.h>

size_t height, width;
bool window_size_changed = false;

void sig_winch(__attribute__((unused)) int irq) {
    window_size_changed = true;
}

void window_init() { 
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

    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, -1);
    init_pair(3, COLOR_BLUE, COLOR_BLUE);
}

void draw_status_bar() {
    attron(COLOR_PAIR(1));
    mvaddch(height - 2, 0, ' ');
    char status[] = "Status";
    mvaddstr(height - 2, 1, status);
    for (size_t i = sizeof(status); i < width; i++) {
        mvaddch(height - 2, i, ' ');
    }

    attroff(COLOR_PAIR(1));
}



struct winsize get_term_size() {
    struct winsize size;
    ioctl(1, TIOCGWINSZ, &size);
    return size;
}

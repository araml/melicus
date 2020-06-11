#include <lyrics.h>
#include <sys/ioctl.h>
#include <ncurses.h>

int main(int argc, char *argv[]) {
    // initializes screen
    // set ups memory and clear screen
    lyrics_t *lyrics = create_lyrics();
    load_lyric_from_file(lyrics, "test.lyric");

    //printf("Lyrics:\n %s", lyrics->lyrics);

    initscr();

    struct winsize max;
    ioctl(1, TIOCGWINSZ, &max);
    //getmaxyx(0, height, width);
    int height = 3;
    printf("width: %d, height %d\n", max.ws_row, max.ws_col);
    //    mvprintw(height, init_pos, "%s", l.c_str());

    nodelay(stdscr, true);

    char *line;
    while (true) {
        char *line = next_line(lyrics);
        if (line == 0) {
            break;
        }

        //printf("%s\n", line);
        int init_pos = center_text(length(line), max.ws_col);
        mvprintw(height, init_pos, "%s", line);
        height++;
    }

    while (true) {
        if (getch() != ERR) {
            break;
        }
    }

    //getch();
    // refresh(); // Refreshes the screen to match whats in memory

    endwin(); // Frees all ncurses related stuff*/

    free_lyrics(lyrics);
    return 0;
}

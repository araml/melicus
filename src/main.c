#include <lyrics.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ncurses.h>


const char *status_path = "/home/maek/cmus-status.txt";


int get_line(int fd, char **sline) {
    size_t reserved_size = 1;
    size_t used_size = 0;

    // TODO: Fix this, on several calls is trashing previous memory
    *sline = (char *)malloc(1);
    char *line = *sline;
    char c;
    int status;
    while (true) {
        status = read(fd, &c, 1);

        if (status == 0 || status == -1)
            return status;

        if (c == '\n') {
            if (reserved_size == used_size) {
                realloc(*sline, reserved_size + 1);
            } else {
                realloc(*sline, used_size + 1);
            }

            line[used_size] = '\0';
            return 1;
        }

        if (reserved_size == used_size) {
            reserved_size *= 2;
            realloc(*sline, reserved_size);
        }

        line[used_size++] = c;
    }
}

char *load_name_from_status_file(const char *path) {
    int status_fd = open(path, O_RDONLY);
    char *line = NULL;
    while (get_line(status_fd, &line) != 0) {}

    return line;
}

int main(int argc, char *argv[]) {
    // initializes screen
    // set ups memory and clear screen
    lyrics_t *lyrics = create_lyrics();
    load_lyric_from_file(lyrics, "test.lyric");
    char *status = load_name_from_status_file(status_path);
    printf("Status\n%s\n", status);


    //printf("Lyrics:\n %s", lyrics->lyrics);

    return 0;

    initscr();

    struct winsize max;
    ioctl(1, TIOCGWINSZ, &max);
    //getmaxyx(0, height, width);
    int height = 3;
    printf("width: %d, height %d\n", max.ws_row, max.ws_col);
    //    mvprintw(height, init_pos, "%s", l.c_str());

    nodelay(stdscr, true);

    /*
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
    */

    //int status_file = open(status_path, O_RDONLY);

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

#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define BLOCKSIZE 4096

typedef struct {
    char *lyrics;
    size_t size; // total size
    size_t reserved_size;
    size_t cl_idx; // line index (in the whole file)
    char *cl; // current `line`
} lyrics_t;

lyrics_t *create_lyrics() {
    lyrics_t *lyrics = (lyrics_t *)malloc(sizeof(lyrics_t));
    lyrics->lyrics = NULL;
    lyrics->size = 0;
    lyrics->cl_idx = 0;
    lyrics->cl = (char *)malloc(BLOCKSIZE);
    return lyrics;
}

void free_lyrics(lyrics_t *lyrics) {
    free(lyrics->cl);
    free(lyrics->lyrics);
}

size_t length(const char *line) {
    size_t len = 0;
    for (; line && line[len] != '\0'; len++) {}
    return len;
}

void load_lyric_from_file(lyrics_t *lyrics, const char *path) {
    if (!lyrics) {
        return;
    }

    int lyric_fd = open(path, O_RDONLY);
    size_t position = 0;
    ssize_t status = 0;




    while (true) {
        char *tmp_buf = (char *)malloc(BLOCKSIZE + lyrics->size);
        for (size_t i = 0; i < lyrics->size; i++)
            tmp_buf[i] = lyrics->lyrics[i];
        free(lyrics->lyrics);
        lyrics->lyrics = tmp_buf;

        status = read(lyric_fd, lyrics->lyrics + position, BLOCKSIZE);
        if (status == -1) {
            perror("Error loading lyrics");
            return;
        }
        if (status == 0)
            break;


        position += status;
        lyrics->size += status;
    }
}

char* next_line(lyrics_t *lyrics) {
    if (lyrics->cl_idx >= lyrics->size)
        return 0;

    for (size_t i = 0; i < BLOCKSIZE; i++) {
        lyrics->cl[i] = '\0';
    }

    for (size_t k = 0, i = lyrics->cl_idx;
        lyrics->lyrics[i] != '\n'; i++, k++) {
        lyrics->cl[k] = lyrics->lyrics[i];
    }

    lyrics->cl_idx += length(lyrics->cl) + 1; // +1 for the '\n'
    return lyrics->cl;
}

int center(int text_width, int screen_width) {
    screen_width = screen_width >> 1;
    text_width = screen_width >> 1;
    return screen_width - text_width;
}

int main(int argc, char *argv[]) {
    // initializes screen
    // set ups memory and clear screen
    lyrics_t *lyrics = create_lyrics();
    load_lyric_from_file(lyrics, "test.lyric");

    //printf("Lyrics:\n %s", lyrics->lyrics);

    initscr();

    struct winsize max;
    ioctl(0, TIOCGWINSZ, &max);
    // getmaxyx(0, height, width);
    int height = 3;

    //int init_pos = center(l.length(), max.ws_row);
    //    mvprintw(height, init_pos, "%s", l.c_str());

    char *line;
    while (true) {
        char *line = next_line(lyrics);
        if (line == 0) {
            break;
        }

        //printf("%s\n", line);
        mvprintw(height, 4, "%s", line);
        height++;
    }

    getch();
    // refresh(); // Refreshes the screen to match whats in memory

    endwin(); // Frees all ncurses related stuff*/

    free_lyrics(lyrics);
    return 0;
}

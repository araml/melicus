#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    char *lyrics;
    size_t size; // total size
    // save the state that it has reserved memory for a line
    int reserved_line;
} lyrics_t;

lyrics_t *create_lyrics() {
    lyrics_t *lyrics = (lyrics_t *)malloc(sizeof(lyrics_t));
    lyrics->lyrics = NULL;
    lyrics->size = 0;
    return lyrics ;
}

void free_lyrics(lyrics_t *lyrics) {
    free(lyrics->lyrics);
}

size_t length(const char *line) {
    size_t len = 0;
    for (; line && line[len] != '\0'; len++) {}
    return len;
}

#define BLOCKSIZE 4096

void load_lyric_from_file(lyrics_t *lyrics, const char *path) {
    if (!lyrics) {
        return;
    }

    int lyric_fd = open(path, O_RDONLY);
    lyrics->lyrics = (char *)malloc(BLOCKSIZE);
    lyrics->size = BLOCKSIZE;
    size_t position = 0;
    ssize_t status = 0;
    while (true) {
        status = read(lyric_fd, lyrics->lyrics + position, BLOCKSIZE);
        if (status == -1)
            perror("Error loading lyrics");
        if (status == 0)
            break;

        char *tmp_buf = (char *)malloc(BLOCKSIZE + lyrics->size);
        for (size_t i = 0; i < lyrics->size; i++)
            tmp_buf[i] = lyrics->lyrics[i];
        free(lyrics->lyrics);

        lyrics->lyrics = tmp_buf;
        position += BLOCKSIZE;
        lyrics->size += BLOCKSIZE;
    }
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
    free_lyrics(lyrics);

    printf("Lyrics:\n %s", lyrics->lyrics);

    /*
    initscr();

    struct winsize max;
    ioctl(0, TIOCGWINSZ, &max);
    // getmaxyx(0, height, width);
    int height = 3;

    //int init_pos = center(l.length(), max.ws_row);
    //    mvprintw(height, init_pos, "%s", l.c_str());

    getch();
    // refresh(); // Refreshes the screen to match whats in memory

    endwin(); // Frees all ncurses related stuff*/
    return 0;
}

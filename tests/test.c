#include <lyrics.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <stdlib.h>

const char *status_path = "/home/maek/cmus-status.txt";

char *read_from_memory() {
    char c;
    size_t line_size = 2;
    char *line = (char *)malloc(2);
    size_t index = 0;
    while (true) {
        int status = read(STDIN_FILENO, &c, 1);
        if (status == -1) {
            perror("Error reading from memory");
            return NULL;
        }

        if ((line_size - 1) == 0) {
            char *tmp = realloc(line, line_size * 2);
            if (tmp == NULL) {
                free(line);
                return NULL;
            }

            if (tmp != line) {
                free(line);
                line = tmp;
            }
            index++;
            line_size *= 2;
        }

        if (status == 0)
            break;
    }
}

int main(int argc, char *argv[]) {
    lyrics_t *lyrics = create_lyrics();

    while (true) {
        char *line = read_from_memory();
        load_lyric_from_memory(lyrics, line);

        while (true) {
            char *line = next_line(lyrics);
            if (line == 0) {
                break;
            }
        }

        free_lyrics(lyrics);
    }

    return 0;
}

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <lyrics.h>

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
    free(lyrics);
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

    // TODO: Right now not accounting for \r\n
    for (size_t k = 0, i = lyrics->cl_idx;
        lyrics->lyrics[i] != '\n'; i++, k++) {
        lyrics->cl[k] = lyrics->lyrics[i];
    }

    lyrics->cl_idx += length(lyrics->cl) + 1; // +1 for the '\n'
    return lyrics->cl;
}

int center_text(int text_width, int term_width) {
    term_width = term_width >> 1;
    text_width = text_width >> 1;
    return term_width - text_width;
}

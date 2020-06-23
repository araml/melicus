#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <lyrics.h>
#include <string_utils.h>

lyrics_t *create_lyrics() {
    lyrics_t *lyrics = (lyrics_t *)malloc(sizeof(lyrics_t));
    lyrics->lyrics = NULL;
    lyrics->size = 0;
    lyrics->cl_idx = 0;
    // TODO: possible lyrics with more than 4k in a single line?
    // TODO: maybe bug if song isnt line break correctly
    lyrics->cl = (char *)malloc(BLOCKSIZE);
    return lyrics;
}

void free_lyrics(lyrics_t *lyrics) {
    free(lyrics->cl);
    free(lyrics->lyrics);
    free(lyrics);
}

// TODO: check if all the loading functions can be merged into a single function
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

void load_lyric_from_memory(lyrics_t *lyrics, const char *memory) {
    size_t sz = 0;
    for (size_t i = 0; memory[i] != '\0'; i++) { sz++; }
    lyrics->size = sz + 1;
    lyrics->lyrics = (char *)malloc(lyrics->size);
    for (size_t i = 0; i < lyrics->size; i++) {
        lyrics->lyrics[i] = memory[i];
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

/* Gonna parse HTML by hand, I know I shouldn't but i don't care because i'm
 * assuming the page will be fixed for a long time
 */

char *get_lyrics_from_page_string(const char *page_string) {
    /* We don't know if we got redirected to a page with the song or not
     * we'll try to match with lyrics first
     * if we can't find any then we'll search for links to any lyrics
     * if then we can't find any then the song isn't in the database.
     */


    // Tries to match the lyrics inside the html
    char opening_div[] = "<div class=\"holder lyric-box\">";
    size_t idx = find_in_string(page_string, opening_div);
    if (idx == (size_t)-1)
        return NULL;

    idx += length(opening_div);

    char *lyrics = calloc(1, 1);
    size_t k = 0;
    size_t lyrics_length = 1;
    size_t page_string_length = length(page_string);
    for (size_t i = idx; i < page_string_length; i++) {
        if (k >= lyrics_length) {
            char *tmp = realloc(lyrics, lyrics_length *= 2);
            if (tmp)
                lyrics = tmp;
        }

        // very ugly
        // basically breaks when it finds the closing <div
        if (page_string[i] == '<' && page_string[i + 1] == 'd')
            break;
        lyrics[k++] = page_string[i];
    }
    lyrics[k] = '\0';
    return lyrics;
}

#ifndef LYRICS_H
#define LYRICS_H

#include <unistd.h>

#define BLOCKSIZE 4096

typedef struct {
    char *lyrics;
    size_t size; // total size
    size_t reserved_size;
    size_t cl_idx; // line index (in the whole file)
    char *cl; // current `line`
} lyrics_t;

lyrics_t *create_lyrics();
void free_lyrics(lyrics_t *lyrics);
size_t length(const char *line);
void load_lyric_from_file(lyrics_t *lyrics, const char *path);
void load_lyric_from_memory(lyrics_t *lyrics, const char *memory);
char *next_line(lyrics_t *lyrics);
int center_text(int text_width, int term_width);
char *get_lyrics_from_page_string(const char *page_string);

#endif // LYRICS_H

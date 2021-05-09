#ifndef SONG_DATA_H
#define SONG_DATA_H

#include <stdbool.h>

typedef struct {
    char *artist_name;
    char *album;
    char *song_name;
    bool is_valid;
} song_data_t;

song_data_t create_song_data();
song_data_t create_invalid_song_data();
void destroy_song_data(song_data_t *d);

#endif // SONG_DATA_H

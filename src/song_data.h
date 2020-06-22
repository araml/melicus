#ifndef SONG_DATA_H
#define SONG_DATA_H

typedef struct {
    char *name;
    char *album;
    char *song_name;
} song_data;

void destroy_song_data(song_data *d);

#endif // SONG_DATA_H

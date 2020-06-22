#include <song_data.h>
#include <string.h>
#include <stdlib.h>

void destroy_song_data(song_data *d) {
    free(d->album);
    free(d->song_name);
    free(d->name);
    free(d);
}



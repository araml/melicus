#include <stdlib.h>
#include <song_data.h>

song_data_t create_song_data() { 
    song_data_t song_data = { 
        .album = NULL,
        .song_name = NULL,
        .artist_name = NULL,
        .is_valid = true,
    };
    
    return song_data;
}


song_data_t create_invalid_song_data() {
    song_data_t song_data = create_song_data();
    song_data.is_valid = false;
    return song_data;
}

void destroy_song_data(song_data_t *d) {
    free(d->album);
    free(d->song_name);
    free(d->artist_name);
}



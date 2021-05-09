#ifndef OPEN_LYRICS_DATABASE_H
#define OPEN_LYRICS_DATABASE_H 

/* Get lyrics from open lyrics database  
 * https://lyrics.github.io/
 */

#include <song_data.h>
#include <string_split.h>

string_split_t oldb_get_lyrics(const song_data_t *s);

#endif // OPEN_LYRICS_DATABASE_H

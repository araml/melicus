#ifndef OPEN_LYRICS_DATABASE_H
#define OPEN_LYRICS_DATABASE_H 

/* Get lyrics from open lyrics database  
 * https://lyrics.github.io/
 */

#include <song_data.h>
#include <string_utils.h>

string_split *oldb_get_lyrics(const song_data *s);

#endif // OPEN_LYRICS_DATABASE_H

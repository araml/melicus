#ifndef SM_H
#define SM_H
/* songmeanings.net crawler */
#include <song_data.h>
#include <string_split.h>

char *sm_make_song_url(song_data_t *data);
char *sm_find_link_for_song(char *page, song_data_t *s);
char *sm_get_lyrics_from_page_string(const char *page_string);
string_split_t sm_clean_lyrics(char *lyrics);
string_split_t sm_get_lyrics(song_data_t *s);

#endif // SM_H

#ifndef SM_H
#define SM_H
/* songmeanings.net crawler */
#include <song_data.h>
#include <string_split.h>

char *sm_make_song_url(song_data *data);
char *sm_find_link_for_song(char *page, song_data *s);
char *sm_get_lyrics_from_page_string(const char *page_string);
string_split *sm_clean_lyrics(char *lyrics);
string_split *sm_get_lyrics(song_data *s);

#endif // SM_H

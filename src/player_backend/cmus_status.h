#ifndef CMUS_STATUS_H
#define CMUS_STATUS_H

#include <stdlib.h>
#include <song_data.h>
#include <string_split.h>

int realloc_wrapper(char **ptr, size_t size);
int get_line(int fd, char **sline);
song_data *load_song_data_cmus(const char *path);
string_split_t get_cmus_status();
song_data *get_current_song();

#endif // CMUS_STATUS_H

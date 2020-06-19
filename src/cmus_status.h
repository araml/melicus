#ifndef CMUS_STATUS_H
#define CMUS_STATUS_H

#include <stdlib.h>
#include <song_data.h>

const char* cmus_status_path;

int realloc_wrapper(char **ptr, size_t size);
int get_line(int fd, char **sline);
song_data *load_song_data_cmus(const char *path);
char *get_cmus_status();

#endif // CMUS_STATUS_H

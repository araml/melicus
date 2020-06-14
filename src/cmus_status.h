#ifndef CMUS_STATUS_H
#define CMUS_STATUS_H

#include <stdlib.h>

const char* cmus_status_path;

int realloc_wrapper(char **ptr, size_t size);
int get_line(int fd, char **sline);
char *load_name_from_status_file(const char *path);

#endif // CMUS_STATUS_H

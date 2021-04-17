#ifndef STRING_SPLIT_H
#define STRING_SPLIT_H

#include <stddef.h>

typedef struct {
    char **strings;
    size_t size;
    size_t reserved_size;
} string_split ;

string_split *create_string_split();
int add_to_string_split(string_split *ss, const char *s);
void destroy_string_split(string_split *s);
int push_to_string_split(string_split *sv, const char *line);

#endif // STRING_SPLIT_H

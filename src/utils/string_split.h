#ifndef STRING_SPLIT_H
#define STRING_SPLIT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char **strings;
    size_t size;
    size_t reserved_size;
    bool is_valid;
} string_split_t;

string_split_t create_string_split();
string_split_t create_invalid_string_split();
void destroy_string_split(string_split_t *s);
int push_to_string_split(string_split_t *sv, const char *line);

#endif // STRING_SPLIT_H

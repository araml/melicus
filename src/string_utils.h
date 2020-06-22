#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

typedef struct {
    char **strings;
    size_t used_size;
    size_t reserved_size;
} string_split ;

size_t length(const char *line);
size_t get_size_until_next_smybol(const char *s, size_t index, char c);
string_split *create_string_string_split(const char *s, char c);
int add_to_string_split(string_split *ss, const char *s);
void destroy_string_split(string_split *s);
int check_prefix(const char *prefix, const char *l);
void if_substring_fill(char **to_fill, const char *prefix, const char *subs);
size_t find_in_string(char *s, char *to_find);

#endif // STRING_UTILS_H

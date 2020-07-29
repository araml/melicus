#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdbool.h>
typedef struct {
    char **strings;
    size_t size;
    size_t reserved_size;
} string_split ;

size_t length(const char *line);
size_t codepoints(const char *line);
size_t get_size_until_next_smybol(const char *s, size_t index, char c);
string_split *create_string_split();
int add_to_string_split(string_split *ss, const char *s);
void destroy_string_split(string_split *s);
int check_prefix(const char *prefix, const char *l);
void if_substring_fill(char **to_fill, const char *prefix, const char *subs);
size_t find_in_string(const char *s, const char *to_find);
int check_suffix(const char *text, const char *word, size_t text_length, size_t word_length);
size_t reverse_find(char *text, char *word, size_t text_length);
bool string_cmp(char *s1, char *s2);
int get_line(int fd, char **sline);

int add_to_string(char **s1, const char *s2);
int add_char_to_string(char **s, char c);

int push_to_string_split(string_split *sv, const char *line);

#endif // STRING_UTILS_H

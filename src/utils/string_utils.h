#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdbool.h>

#define MELICUS_ERR (-1)
#define MELICUS_UNUSED __attribute__((unused))


void if_substring_fill(char **to_fill, const char *prefix, const char *subs);

size_t length(const char *line);
size_t codepoints(const char *line);
int match(const char *s, const char *prefix, size_t s_length);
int check_prefix(const char *prefix, const char *l);
size_t find_in_string(const char *s, const char *to_find);
size_t get_size_until_next_smybol(const char *s, size_t index, char c);
int check_suffix(const char *text, const char *word, size_t text_length, 
                 size_t word_length);

size_t reverse_find(char *text, char *word, size_t text_length);
bool string_cmp(char *s1, char *s2);
int get_line(int fd, char **sline);

int add_to_string(char **s1, const char *s2);
int add_char_to_string(char **s, char c);

// returns 0 if not an ASCII letter (OLDB only);
char uppercase_char(char c);
// TODO(aram): maybe add boolean to trigger this behavior our not? 
// (in that case does it even make sense to uppercase a non-ascii string?)
// returns NULL if there is a non-ASCII letter (OLDB only)
char *uppercase_string(const char *lowercased);


#endif // STRING_UTILS_H

#include <string_utils.h>
#include <string.h>
#include <stdlib.h>

size_t length(const char *line) {
    size_t len = 0;
    for (; line && line[len] != '\0'; len++) {}
    return len;
}

size_t get_size_until_next_smybol(const char *s, size_t index, char c) {
    size_t result = 0;
    for(size_t i = index; i < length(s) && s[i] != c; i++) {
        result++;
    }

    return result;
}

/* splits a string by the character ' ' (generic later)
 * reserves memory for both the string split and every string inside of it
 * used_size is the number of strings allocated while reserved size is the
 * size of the array pointing to each string
 * ALL memory should be free'd with destroy_split_string
 */
string_split *create_string_string_split(const char *s, char c) {
    string_split *ss = (string_split *)(malloc(sizeof(string_split)));
    ss->strings = (char **)malloc(sizeof(char *) * 1);
    ss->used_size = 0;
    ss->reserved_size = 1;

    for (size_t i = 0; i < length(s); i++) {
        if (ss->used_size == ss->reserved_size) {
            char **tmp = (char **)realloc(ss->strings,
                                          sizeof(char *) *
                                          (ss->reserved_size *= 2));
            if (tmp != NULL)
                ss->strings = tmp;
            else return NULL;
        }

        size_t sz = get_size_until_next_smybol(s, i, c);
        if (sz == 0)
            continue;
        ss->strings[ss->used_size] = (char *)malloc(sz + 1);
        memset(ss->strings[ss->used_size], '\0', sz + 1);
        for (size_t k = i; i < length(s) && s[i] != c; i++) {
            ss->strings[ss->used_size][i - k] = s[i];
        }

        ss->used_size++;
    }

    return ss;
}

void destroy_string_split(string_split *ss) {
    for (size_t i = 0; i < ss->used_size; i++)
        free(ss->strings[i]);
    free(ss->strings);
    free(ss);
}

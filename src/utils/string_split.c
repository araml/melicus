#include <stdlib.h>
#include <string.h>
#include <string_split.h>

/* splits a string by the character ' ' (generic later)
 * reserves memory for both the string split and every string inside of it
 * used_size is the number of strings allocated while reserved size is the
 * size of the array pointing to each string
 * ALL memory should be free'd with destroy_split_string
 */
string_split *create_string_split() {
    string_split *r = (string_split *)malloc(sizeof(string_split));
    r->strings = NULL;
    r->size = 0;
    r->reserved_size = 0;
    return r;
}

void destroy_string_split(string_split *ss) {
    if (!ss)
        return;
    for (size_t i = 0; i < ss->size; i++)
        free(ss->strings[i]);
    free(ss->strings);
    free(ss);
}

int push_to_string_split(string_split *ss, const char *line) {
    if (!line)
        return -1;

    if (ss->reserved_size == 0) {
        ss->strings = (char **)malloc(sizeof(char *));
        ss->reserved_size = 1;
    } else if (ss->size == ss->reserved_size) {
        char **tmp = (char **)realloc(ss->strings, sizeof(char *) *
                                      (ss->reserved_size *= 2));
        if (tmp) {
            ss->strings = tmp;
        } else {
            return -1;
        }
    }

    ss->strings[ss->size] = (char *)malloc(length(line) + 1);
    memset(ss->strings[ss->size], '\0', length(line) + 1);
    for (size_t i = 0; i < length(line); i++) {
        ss->strings[ss->size][i] = line[i];
    }

    ss->size++;

    return 0;
}

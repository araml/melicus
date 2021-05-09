#include <stdlib.h>
#include <string.h>
#include <string_split.h>
#include <string_utils.h>

/* splits a string by the character ' ' (generic later)
 * reserves memory for both the string split and every string inside of it
 * used_size is the number of strings allocated while reserved size is the
 * size of the array pointing to each string
 * ALL memory should be free'd with destroy_split_string
 */
string_split_t create_string_split() {
    string_split_t string_split = { 
        .strings = NULL,
        .size = 0,
        .reserved_size = 0,
        .is_valid = true,
    };
    return string_split;
}

string_split_t create_invalid_string_split() {
    string_split_t string_split = { 
        .is_valid = false,
    };
    return string_split;
}

void destroy_string_split(string_split_t *ss) {
    for (size_t i = 0; i < ss->size; i++)
        free(ss->strings[i]);
    free(ss->strings);
}

int push_to_string_split(string_split_t *ss, const char *line) {
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

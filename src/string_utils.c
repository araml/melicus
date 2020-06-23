#include <string_utils.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

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

int add_to_string_split(string_split *ss, const char *s) {
    if (!s)  // NULL string
        return -1;

    if (ss->used_size == ss->reserved_size) {
        char **tmp = (char **)realloc(ss->strings,
                                      sizeof(char *) *
                                      (ss->reserved_size *= 2));
        if (tmp != NULL)
            ss->strings = tmp;
        else return -1;
    }

    ss->strings[ss->used_size] = (char *)malloc(length(s) + 1);
    memset(ss->strings[ss->used_size], '\0', length(s) + 1);
    for (size_t i = 0; i < length(s); i++) {
        ss->strings[ss->used_size][i] = s[i];
    }

    ss->used_size++;
    return 0;
}

void destroy_string_split(string_split *ss) {
    for (size_t i = 0; i < ss->used_size; i++)
        free(ss->strings[i]);
    free(ss->strings);
    free(ss);
}

int check_prefix(const char *prefix, const char *s) {
    size_t prefix_length = length(prefix);
    size_t s_length = length(s);
    for (size_t i = 0; i < prefix_length && i < s_length; i++) {
        // CMUS_remote q data starts after the tag word, maybe regex it?
        if (prefix[i] != s[i])
            return 0;
    }

    return 1;
}

void if_substring_fill(char **to_fill, const char *prefix, const char *subs) {
    if (!check_prefix(prefix, subs + 4)) {
        return;
    }

    //TODO: allocating a few bytes more here but meh
    *to_fill = (char *)malloc(length(subs));
    memset(*to_fill, 0, length(subs));
    // hardcoding part of the cmus struct "tag album ALBUM_NAME"
    // 3 letters for tag, 2 spaces and the word album/artist/song

    memcpy(*to_fill, subs + length(prefix) + 5,
           length(subs) - length(prefix) - 5 + 1);
}

int check_prefix_with_length(const char *prefix, const char *s, size_t prefix_length,
        size_t s_length) {
    for (size_t i = 0; i < prefix_length && i < s_length; i++) {
        // CMUS_remote q data starts after the tag word, maybe regex it?
        if (prefix[i] != s[i])
            return 0;
    }

    return 1;

}

// TODO: VERY NAIVE NOW, should fix with a real algorithm later.
size_t find_in_string(const char *s, const char *to_find) {
    size_t prefix_length = length(to_find);
    size_t s_length = length(s);
    for (size_t i = 0; i <= s_length - prefix_length; i++) {
        if (check_prefix_with_length(to_find, s + i, s_length - i, prefix_length)) {
            return i;
        }
    }
    return -1;
}

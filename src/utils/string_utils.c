#define _XOPEN_SOURCE 500
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <string_utils.h>

size_t length(const char *line) {
    size_t len = 0;
    for (; line && line[len] != '\0'; len++) {}
    return len;
}

// TODO: Add proper checking of valid utf8
size_t codepoints(const char *line) {
    size_t len = 0;
    size_t codepoints = 0;
    for (; line && line[len] != '\0';) {
        if ((line[len] & 0xF0) == 0xF0) {
            len += 4;
            codepoints++;
            continue;
        }
        if ((line[len] & 0xE0) == 0xE0) {
            len += 3;
            codepoints++;
            continue;
        }

        if ((line[len] & 0xC0) == 0xC0) {
            len += 2;
            codepoints++;
            continue;
        }

        if ((line[len] & 0x80) != 0x80) {
            len++;
            codepoints++;
            continue;
        }
    }

    return codepoints;
}


size_t get_size_until_next_smybol(const char *s, size_t index, char c) {
    size_t result = 0;
    for(size_t i = index; i < length(s) && s[i] != c; i++) {
        result++;
    }

    return result;
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

int match(const char *lyrics, const char *prefix, size_t s_length) { 
    size_t prefix_length = length(prefix);
    if (s_length < prefix_length)
        return 0;

    for (size_t i = 0; i < prefix_length; i++) {
        if (prefix[i] != lyrics[i])
            return 0;
    }

    return 1;
} 


// TODO: this is cmus specific, maybe move there?
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

int check_suffix(const char *text, const char *word, size_t text_length, size_t word_length) {
    for (size_t i = 0; i <= word_length && i <= text_length; i++) {
        if (text[text_length - i] != word[word_length - i])
            return 0;
    }

    return 1;
}

size_t reverse_find(char *text, char *word, size_t text_length) {
    size_t word_length = length(word);

    for (size_t i = word_length; i < text_length; i++) {
        if (check_prefix_with_length(text - i, word, word_length, word_length)) {
            return i;
        }
    }

    return 0;
}

bool string_cmp(char *s1, char *s2) {
    if (!s1 || !s2)
        return false;

    while (*s1 != '\0' && *s2 != '\0') {
        if (*(s1++) != *(s2++))
            return false;
    }

    return true;
}

int realloc_wrapper(char **ptr, size_t size) {
    char *tmp = (char *)realloc(*ptr, size);
    if (tmp == NULL)
        return -1;

    *ptr = tmp;

    return 0;
}

int get_line(int fd, char **sline) {
    size_t reserved_size = 1;
    size_t used_size = 0;

    char c;
    int status;
    while (true) {
        status = read(fd, &c, 1);

        if (status == 0 || status == -1)
            return status;

        if (used_size == 0 && reserved_size == 1) {
            *sline = (char *) malloc(1);
            memset(*sline, 0, 1);
      //      line = *sline;
        }

        if (c == '\n') {
            if (reserved_size == used_size) {
                realloc_wrapper(sline, reserved_size + 1);
            } else {
                realloc_wrapper(sline, used_size + 1);
            }

            (*sline)[used_size] = '\0';
            off_t curr_offset = lseek(fd, 0, SEEK_CUR);
            ssize_t st = pread(fd, &c, 1, curr_offset);
            if (st == 0)
                return 0;
            return 1;
        }

        if (reserved_size == used_size) {
            reserved_size *= 2;
            realloc_wrapper(sline, reserved_size);
        //    line = *sline;
        }

        (*sline)[used_size] = c;
        used_size++;
    }
}

int add_to_string(char **s1, const char *s2) {
    if (!s2)
        return -1;

    if (!(*s1)) {
        *s1 = (char *)malloc(length(s2) + 1);
        memcpy(*s1, s2, length(s2) + 1);
    } else {
        size_t l = length(*s1);
        char *tmp = (char *)realloc(*s1, l + length(s2) + 1);
        if (tmp) {
            *s1 = tmp;
            memcpy(*s1 + l, s2, length(s2) + 1);
        } else {
            return -1;
        }
    }

    return 0;
}

// TODO: Fix overly large realloc (exponential)
// not checking current length...
int add_char_to_string(char **s, char c) {
    if (*s == NULL) {
        *s = (char *)malloc(2);
        (*s)[0] = c;
        (*s)[1] = '\0';
    } else {
        char *tmp = (char *)realloc(*s, length(*s) + 2);
        if (tmp)
            *s = tmp;
        else
            return -1;
        size_t s_length = length(*s);
        (*s)[s_length] = c;
        (*s)[s_length + 1] = '\0';
    }

    return 0;
}



char uppercase_char(char c) { 
   if (c >=  65 && c <= 90)
       return c;
   else if (c >= 97 && c <= 122)
       return c - 32;
   return 0;
}

char *uppercase_string(const char *lowercased) { 
    size_t size = length(lowercased) + 1;
    char *uppercased = malloc(size);
    memset(uppercased, 0, size);
    int token = 1;
    
    for (size_t i = 0; i < size; i++) { 
        if (token && uppercased[i] != ' ' && uppercased[i] != '\0') {
            char c = uppercase_char(lowercased[i]);
            if (!c) 
                return NULL; // error non-ascii (OLDB only)
            uppercased[i] = c;
            token = 0;
        } else { 
            uppercased[i] = lowercased[i];
        }

        if (uppercased[i] == ' ') {
            token = 1;
        }
    }

    return uppercased;
}

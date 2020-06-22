#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include <cmus_status.h>
#include <song_data.h>
#include <string_utils.h>

#include <stdio.h>

const char *cmus_status_path = "/home/maek/cmus-status.txt";
// CMUS Tags
const char artist[] = "artist";
const char album[] = "album";
const char song_title[] = "title";

int realloc_wrapper(char **ptr, size_t size) {
    char *tmp = realloc(*ptr, size);
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

string_split* get_cmus_status() {
    int pipe_fd[2]; // Read from 0 write to 1
    pipe(pipe_fd);

    int pid = fork();
    if (pid == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        char bin[] = "/usr/bin/cmus-remote";
        char *args[3] = {"cmus-remote", "-Q", NULL};
        execvp(bin, args);
        return NULL;
    } else {
        close(pipe_fd[1]);
        char *line = NULL;

        string_split *ss = create_string_string_split(NULL, '~');

        while (get_line(pipe_fd[0], &line) != 0) {
            add_to_string_split(ss, line);
            free(line);
        }
        return ss;
    }
}

song_data *get_current_song() {
    string_split *ss = get_cmus_status();
    if (ss->used_size == 0) {
        return NULL;
    }

    song_data *s = (song_data *)malloc(sizeof(song_data));
    memset(s, 0, sizeof(song_data));

    for (size_t i = 0; i < ss->used_size; i++) {
        if_substring_fill(&(s->album), album, ss->strings[i]);
        if_substring_fill(&(s->song_name), song_title, ss->strings[i]);
        if_substring_fill(&(s->name), artist, ss->strings[i]);

        if (s->name && s->song_name && s->album)
            break;
    }

    destroy_string_split(ss);
    return s;
}

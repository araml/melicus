#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>

#include <log.h>
#include <cmus_status.h>
#include <song_data.h>
#include <string_utils.h>


const char *cmus_status_path = "/home/maek/cmus-status.txt";
// CMUS Tags
const char artist[] = "artist";
const char album[] = "album";
const char song_title[] = "title";

string_split* get_cmus_status() {
    int pipe_fd[2]; // Read from 0 write to 1
    int pipe_err[2]; // Pipe to read stderr
    pipe(pipe_fd);
    pipe(pipe_err);

    int pid = fork();

    if (pid == 0) {
        close(pipe_fd[0]);
        close(pipe_err[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_err[1], STDERR_FILENO);
        char bin[] = "/usr/bin/cmus-remote";
        char *args[3] = {"cmus-remote", "-Q", NULL};
        execvp(bin, args);
        return NULL;
    } else {
        close(pipe_fd[1]);
        close(pipe_err[1]);
        char *line = NULL;

        string_split *ss = create_string_split();

        while (get_line(pipe_fd[0], &line) != 0) {
            push_to_string_split(ss, line);
            free(line);
        }

        // We didn't read nothing from stdout cmus is closed
        if (ss->size == 0) {
            while (true) {
                ssize_t err  = get_line(pipe_err[0], &line);
                if (err == 0 || err == -1) {
                    break;
                }
                //LOG("Err %s\n", line);
                free(line);
            }
        }

        waitpid(pid, NULL, 0);
        close(pipe_fd[0]);
        close(pipe_err[0]);
        usleep(10000);
        return ss;
    }
}

song_data *get_current_song() {
    string_split *ss = get_cmus_status();
    if (ss->size == 0) {
        return NULL;
    }

    song_data *s = (song_data *)malloc(sizeof(song_data));
    memset(s, 0, sizeof(song_data));

    for (size_t i = 0; i < ss->size; i++) {
        if_substring_fill(&(s->album), album, ss->strings[i]);
        if_substring_fill(&(s->song_name), song_title, ss->strings[i]);
        if_substring_fill(&(s->artist_name), artist, ss->strings[i]);

        if (s->artist_name && s->song_name && s->album)
            break;
    }

    destroy_string_split(ss);
    return s;
}

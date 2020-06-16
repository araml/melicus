#include <ncurses.h>
#include <curl/curl.h>

#include <lyrics.h>
#include <cmus_status.h>
#include <string_utils.h>
#include <string.h>

char* load_cmus_status() {
    int pipe_fd[2]; // Read from 0 write to 1
    pipe(pipe_fd);


    int pid = fork();
    if (pid == 0) {
        printf("hmm\n");
        close(pipe_fd[0]);
        printf("hmm\n");
        dup2(pipe_fd[1], STDOUT_FILENO);
        char bin[] = "/usr/bin/cmus-remote";
        char *args[3] = {"cmus-remote", "-Q", NULL};
        execvp(bin, args);
    } else {
        close(pipe_fd[1]);
        char *line = NULL;
        while (get_line(pipe_fd[0], &line) != 0) {
            printf("%s\n", line);
            free(line);
        }
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    // initializes screen
    // set ups memory and clear screen
    lyrics_t *lyrics = create_lyrics();
    load_lyric_from_file(lyrics, "test.lyric");
    char *status = NULL;
    status = load_cmus_status();

    string_split *ss = create_string_string_split(status, '-');
    printf("Status split\n");
    for (size_t i = 0; i < ss->used_size; i++)
        printf("%s\n", ss->strings[i]);
    destroy_string_split(ss);

    free(status);
    free_lyrics(lyrics);
    return 0;
}

/*
 *
     initscr();

    struct winsize max;
    ioctl(1, TIOCGWINSZ, &max);
    //getmaxyx(0, height, width);
    int height = 3;
    printf("width: %d, height %d\n", max.ws_row, max.ws_col);
    //    mvprintw(height, init_pos, "%s", l.c_str());

    nodelay(stdscr, true);

  //
  //  char *line;
  //  while (true) {
  //      char *line = next_line(lyrics);
  //      if (line == 0) {
  //          break;
  //      }
//
  //      //printf("%s\n", line);
  //      int init_pos = center_text(length(line), max.ws_col);
  //      mvprintw(height, init_pos, "%s", line);
  //      height++;
  //  }


    //int status_file = open(status_path, O_RDONLY);

    while (true) {
        if (getch() != ERR) {
            break;
        }
    }

    //getch();
    // refresh(); // Refreshes the screen to match whats in memory

    endwin(); // Frees all ncurses related stuff/

    free_lyrics(lyrics);*/

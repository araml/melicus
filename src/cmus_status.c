#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include <cmus_status.h>

const char *cmus_status_path = "/home/maek/cmus-status.txt";

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

    // TODO: Fix this, on several calls is trashing previous memory
    //char *line = NULL;
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

char *load_name_from_status_file(const char *path) {
    int status_fd = open(path, O_RDONLY);
    char *line = NULL;
    while (get_line(status_fd, &line) != 0) {
        free(line);
    }

    close(status_fd);
    return line;
}


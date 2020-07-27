#include <log.h>

#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

int fd = 0;

void log_init() {
#if defined(DEBUG)
    fd = open("melicus.log", O_CREAT | O_APPEND | O_WRONLY,
                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                 S_IROTH | S_IWOTH);
#endif
}

void log_melicus(const char *format, ...) {
#if defined(DEBUG)
    va_list arg;
    va_start(arg, format);
    vdprintf(fd, format, arg);
    va_end(arg);
#endif
}

#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static int log_fd = -1;

void my_log(const char* fmt, ...) {
    if(log_fd == -1) {
        log_fd = open("./my_log", O_WRONLY | O_CREAT, 0777);
        if(log_fd < 0) {
            fprintf(stderr, "Error occured when opening ./my_log\n");
            exit(-1);
        }
    }
    va_list ap;
    va_start(ap, fmt);

    char* tmp;
    vasprintf(&tmp, fmt, ap);
    write(log_fd, tmp, strlen(tmp));
    free(tmp);

    va_end(ap);
}

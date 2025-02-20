#include "print.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <time.h>

void animated_print(char const *const content) {
#ifndef DEBUG
        size_t len = strlen((void *)content);
        for (int i = 0; i + 1 < len; ++i) {
                printf("%c", content[i]);
                fflush(stdout);
                thrd_sleep(&(struct timespec){.tv_nsec = 4 * MS}, NULL);
        }
#endif
}

void write_file(char const *const filename, char const *const data) {
        FILE *fp = fopen(filename, "w");
        fwrite(data, 1, strlen(data), fp);
        if (ferror(fp)) {
                fprintf(stderr, "ERROR: %s\n", strerror(errno));
        }
        fclose(fp);
}

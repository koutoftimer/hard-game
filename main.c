#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>

int const NS = 1000;
int const MS = 1000 * 1000;

char const *const CLEAR_SCREEN_ANSI = "\033[1;1H\033[2J";

void animated_print(char const *const);

int main(int argc, char *argv[]) {
        puts(CLEAR_SCREEN_ANSI);
        static char const wellcome_message[] = {
#embed "./data/wellcome_message.txt"
        };
        animated_print(wellcome_message);
        int input = getc(stdin);
        if (input != 10) {
                return EXIT_SUCCESS;
        }
        puts(CLEAR_SCREEN_ANSI);
        return EXIT_SUCCESS;
}

void animated_print(char const *const content) {
        size_t len = strlen((void *)content);
        for (int i = 0; i < len; ++i) {
                putc(content[i], stdout);
                fflush(stdout);
                thrd_sleep(&(struct timespec){.tv_nsec = 4 * MS}, NULL);
        }
}

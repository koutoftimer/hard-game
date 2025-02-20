#define IMPLEMENTATION
#include "../strategy.h"

#include <git2.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/repository.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../print.h"

char const *const intro_message() {
        static char message[] = {
#embed "intro.txt"
        };
        return message;
}

void strategy() {
        puts(CLEAR_SCREEN_ANSI);
        animated_print(intro_message());
        printf("\n");

#ifndef DEBUG
        int input = getc(stdin);
        if (input != 10) {  // If not Enter => Exit
                exit(EXIT_SUCCESS);
        }
#endif
        puts(CLEAR_SCREEN_ANSI);
}

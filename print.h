#pragma once

#include <assert.h>
#define TODO(msg) assert(0 && msg)

#define NS (1000)
#define MS (1000 * 1000)
#define CLEAR_SCREEN_ANSI "\033[1;1H\033[2J"

#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

void animated_print(char const *const content);
void write_file(char const *const, char const *const);

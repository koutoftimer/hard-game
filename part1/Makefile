CFLAGS  = -Wall -Werror -pedantic -O2 -ggdb -std=c23 -fPIC -shared
CC = clang

all: core

core: ./core.c
	$(CC) $(CFLAGS) -o core.so core.c

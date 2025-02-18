CFLAGS = -Wall -Werror -pedantic -O2 -ggdb -std=c23
CC = clang

all: main

main: ./main.c embed.h
	$(CC) $(CFLAGS) -o main main.c

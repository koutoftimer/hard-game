CFLAGS  = -Wall -Werror -pedantic -O2 -ggdb -std=c23 -fPIC
LDFLAGS = `pkg-config --libs libgit2`
CC = clang

all: print.o main part0 part1

main: ./main.c
	$(CC) $(CFLAGS) -o main print.o $(LDFLAGS) main.c

print.o: print.c print.h
	$(CC) $(CFLAGS) -c print.c -o print.o

part0: ./part0/*.c ./part0/*.txt print.o
	$(CC) $(CFLAGS) -shared -o ./part0/strategy.so print.o ./part0/strategy.c

part1: ./part1/*.h ./part1/*.c ./part1/*.txt ./part1/Makefile print.o ./part1/simulation.o
	$(CC) $(CFLAGS) -shared -o ./part1/strategy.so print.o ./part1/simulation.o ./part1/strategy.c

./part1/simulation.o: ./part1/simulation.c ./part1/simulation.h
	$(CC) $(CFLAGS) -c ./part1/simulation.c -o ./part1/simulation.o

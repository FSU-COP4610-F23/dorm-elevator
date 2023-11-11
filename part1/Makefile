CC = gcc
STRACE = strace

all: empty part1

empty: empty.c
	$(CC) -o empty empty.c
	$(STRACE) -o empty.trace ./empty

part1: part1.c
	$(CC) -o part1 part1.c
	$(STRACE) -o part1.trace ./part1

clean:
	rm -f empty part1

.PHONY: all clean

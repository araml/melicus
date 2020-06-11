.PHONY: main
all: main

main:
	clang src/main.c src/lyrics.c -lncurses -o main -std=c99 -ggdb -Isrc/

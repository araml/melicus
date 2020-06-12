.PHONY: main
release: main

fuzz: main_fuzz

main:
	clang src/main.c src/lyrics.c -lncurses -o main -std=c99 -ggdb -Isrc/

main_fuzz:
	afl-gcc tests/test.c src/lyrics.c -lncurses -o main_fuzzable -std=c11 -ggdb -D_GNU_SOURCE -Isrc/

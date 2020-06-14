.PHONY: main
release: main

fuzz: main_fuzz

main:
	gcc src/main.c src/lyrics.c -lncurses -o build/main -std=c11 -g -Isrc/

main_fuzz:
	afl-gcc tests/test.c src/lyrics.c -lncurses -o build/main_fuzzable -std=c11 -ggdb -D_GNU_SOURCE -Isrc/

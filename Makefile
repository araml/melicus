CC=gcc
FUZZ_CC=afl-gcc

FLAGS=-std=c11 -ggdb
LIBS= -lcurl -lncurses
INCLUDE=-Isrc/

.PHONY: main
release: main

fuzz: main_fuzz

main:
	$(CC) src/main.c src/lyrics.c src/cmus_status.c src/string_utils.c $(LIBS) $(FLAGS) $(INCLUDE) -o build/main

main_fuzz:
	$(FUZZ_CC) tests/test.c src/lyrics.c -lncurses $(FLAGS) $(INCLUDE) -o build/main_fuzzable

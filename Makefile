CC=gcc
FUZZ_CC=afl-gcc

FLAGS=-std=c11 -ggdb
LIBS= -lcurl -lncurses
INCLUDE=-Isrc/

.PHONY: melicus
all: melicus
fuzz: main_fuzz
string: string_utils
string_search: string_search_test

melicus:
	$(CC) src/main.c src/lyrics.c src/cmus_status.c src/string_utils.c src/network.c src/song_data.c $(LIBS) $(FLAGS) $(INCLUDE) -o build/melicus

main_fuzz:
	$(FUZZ_CC) tests/test.c src/lyrics.c -lncurses $(FLAGS) $(INCLUDE) -o build/main_fuzzable

string_utils:
	$(CC) tests/test_string_utils.c src/string_utils.c $(FLAGS) $(INCLUDE) -o build/string_util

string_search_test:
	$(CC) tests/test_searching_for_string.c src/string_utils.c src/lyrics.c $(FLAGS) $(INCLUDE) -o build/string_search

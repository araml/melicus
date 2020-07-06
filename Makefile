CC=gcc
FUZZ_CC=afl-gcc

FLAGS=-std=c11 --coverage -ggdb -Wall -Wunused-function -Wextra
LIBS= -lcurl -lncursesw
INCLUDE= -Isrc/networking -Isrc/player_backend -Isrc/lyrics -Isrc/utils
SRC = src/networking/network.c src/player_backend/cmus_status.c \
      src/utils/string_utils.c src/lyrics/lyrics.c src/lyrics/song_data.c \
      src/utils/log.c
.PHONY: melicus
all: melicus
#fuzz: main_fuzz
#string: string_utils
#string_search: string_search_test


melicus:
	$(CC) src/main.c $(SRC) $(LIBS) $(FLAGS) $(INCLUDE) -o build/melicus

#main_fuzz:
#	$(FUZZ_CC) tests/test.c src/lyrics.c -lncurses $(FLAGS) $(INCLUDE) -o build/main_fuzzable

#string_utils:
#	$(CC) tests/test_string_utils.c src/string_utils.c $(FLAGS) $(INCLUDE) -o build/string_util

#string_search_test:
#	$(CC) tests/test_searching_for_string.c src/string_utils.c src/lyrics.c $(FLAGS) $(INCLUDE) -o build/string_search

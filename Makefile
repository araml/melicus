CC=gcc
FUZZ_CC=afl-gcc

FLAGS=-std=c11 -ggdb -Wall -Wunused-function -Wextra --coverage
LIBS= -lcurl -lncursesw

NETWORKING = networking
PLAYER_BACKEND = player_backend
UTILS = utils
LYRICS = lyrics
PAGES = pages
INCLUDE_DIRS = $(NETWORKING) $(PLAYER_BACKEND) $(UTILS) $(LYRICS) $(PAGES)
INCLUDE= $(foreach p, $(INCLUDE_DIRS), -Isrc/$p)

SRC = networking/network.c player_backend/cmus_status.c \
      utils/string_utils.c lyrics/lyrics.c lyrics/song_data.c \
      utils/log.c pages/sm.c main.c

OBJS = $(addprefix $(BUILD)/, $(addsuffix .o, $(basename $(SRC))))
FOLDERS = $(sort $(addprefix $(BUILD)/, $(dir $(SRC))))

BUILD = build

.PHONY: melicus clean
all: melicus

$(BUILD)/%.o: %.c
	$(CC) -c $^ -o $@ $(INCLUDE) $(FLAGS)

$(FOLDERS):
	mkdir -p $@

melicus: $(FOLDERS) $(OBJS)
	$(CC) $(OBJS) $(LIBS) $(FLAGS) $(INCLUDE) -o build/melicus

clean:
	rm -rf build/*

vpath %.c src


#fuzz: main_fuzz
#string: string_utils
#string_search: string_search_test

#main_fuzz:
#	$(FUZZ_CC) tests/test.c src/lyrics.c -lncurses $(FLAGS) $(INCLUDE) -o build/main_fuzzable

#string_utils:
#	$(CC) tests/test_string_utils.c src/string_utils.c $(FLAGS) $(INCLUDE) -o build/string_util

#string_search_test:
#	$(CC) tests/test_searching_for_string.c src/string_utils.c src/lyrics.c $(FLAGS) $(INCLUDE) -o build/string_search

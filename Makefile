.PHONY: main
all: main

main:
	clang main.c -lncurses -o main -std=c99

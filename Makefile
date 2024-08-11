CC = gcc
CFLAGS = -std=c90 -Wall -Wextra -pedantic -Isrc -Ibuild

DEBUG_CFLAGS = -g
RELEASE_CFLAGS = -O3

LEX = flex
PAR = bison

BUILD = build
SRC = src
SRCS = texalc.c
OBJS = parser.o lexer.o
EXE = texalc

all: setup build

.PHONY: setup
setup:
	mkdir -p $(BUILD)

.PHONY: build
build: $(SRC)/*
	$(PAR) -d -o $(BUILD)/parser.c $(SRC)/parser.y
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c $(BUILD)/parser.c -o $(BUILD)/parser.o
	$(LEX) -o $(BUILD)/lexer.c $(SRC)/lexer.l
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c $(BUILD)/lexer.c -o $(BUILD)/lexer.o
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $(SRC)/texalc.c $(BUILD)/parser.o $(BUILD)/lexer.o -o texalc

.PHONY: debug
debug: $(SRC)/*
	$(PAR) -d -o $(BUILD)/parser.c $(SRC)/parser.y
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $(BUILD)/parser.c -o $(BUILD)/parser.o
	$(LEX) -o $(BUILD)/lexer.c $(SRC)/lexer.l
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $(BUILD)/lexer.c -o $(BUILD)/lexer.o
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SRC)/texalc.c $(BUILD)/parser.o $(BUILD)/lexer.o -o texalc

.PHONY: clean
clean:
	rm -rf $(BUILD)
	rm -f texalc

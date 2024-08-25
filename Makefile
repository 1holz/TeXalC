BUILD = build
SRC = src
OBJS = parser.o lexer.o numbers.o texalc.o
EXE = texalc
DEBUG_EXE = $(EXE)-debug

RELEASE_DIR = $(BUILD)/release
DEBUG_DIR = $(BUILD)/debug

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -I$(SRC) -Wno-alloc-size-larger-than

RELEASE_CFLAGS = -Ofast -I$(RELEASE_DIR)
DEBUG_CFLAGS = -std=c99 -D_POSIX_VERSION=200112L -D_POSIX_C_SOURCE=200112L -g -O0 -I$(DEBUG_DIR)

LEX = flex
PAR = bison

LEX_FLAGS = --warn
PAR_FLAGS = --header -Wall -Wdangling-alias -Wcounterexamples -fcaret -ffixit
RELEASE_LEX_FLAGS =
DEBUG_LEX_FLAGS = --debug --perf-report --perf-report --nodefault --posix# --lex-compat --verbose
RELEASE_PAR_FLAGS =
DEBUG_PAR_FLAGS = --debug --yacc# --report=all --graph

all: setup release

.PHONY: setup
setup:
	mkdir -p $(RELEASE_DIR)
	mkdir -p $(DEBUG_DIR)

.PHONY: release
release: $(EXE)

$(RELEASE_DIR)/parser.o: $(SRC)/parser.y
	$(PAR) $(PAR_FLAGS) $(RELEASE_PAR_FLAGS) -o $(RELEASE_DIR)/parser.c $(SRC)/parser.y
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c $(RELEASE_DIR)/parser.c -o $(RELEASE_DIR)/parser.o

$(RELEASE_DIR)/lexer.o: $(SRC)/lexer.l
	$(LEX) $(LEX_FLAGS) $(RELEASE_LEX_FLAGS) -o $(RELEASE_DIR)/lexer.c $(SRC)/lexer.l
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c $(RELEASE_DIR)/lexer.c -o $(RELEASE_DIR)/lexer.o

$(RELEASE_DIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c  $< -o $@

$(EXE): $(addprefix $(RELEASE_DIR)/, $(OBJS))
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $^ -o $(EXE)

.PHONY: debug
debug: $(DEBUG_EXE)

$(DEBUG_DIR)/parser.o: $(SRC)/parser.y
	$(PAR) $(PAR_FLAGS) $(DEBUG_PAR_FLAGS) -o $(DEBUG_DIR)/parser.c $(SRC)/parser.y
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $(DEBUG_DIR)/parser.c -o $(DEBUG_DIR)/parser.o

$(DEBUG_DIR)/lexer.o: $(SRC)/lexer.l
	$(LEX) $(LEX_FLAGS) $(DEBUG_LEX_FLAGS) -o $(DEBUG_DIR)/lexer.c $(SRC)/lexer.l
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $(DEBUG_DIR)/lexer.c -o $(DEBUG_DIR)/lexer.o

$(DEBUG_DIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c  $< -o $@

$(DEBUG_EXE): $(addprefix $(DEBUG_DIR)/, $(OBJS))
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $^ -o $(DEBUG_EXE)

.PHONY: clean
clean:
	rm -fr $(RELEASE_DIR)
	rm -fr $(DEBUG_DIR)
	rm -fr $(BUILD)
	rm -fr $(EXE)
	rm -fr $(DEBUG_EXE)

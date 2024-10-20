#    Copyright (C) 2024  Einholz
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published
#    by the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

BUILD = build
SRC = src
OBJS = parser.o lexer.o util.o node.o integer.o
EXE = texalc

YACC = bison
LEX = flex
CC = gcc

YACC_FLAGS = --header -Wall -Wdangling-alias -Wcounterexamples -fcaret -ffixit -Wno-empty-rule
RELEASE_YACC_FLAGS =
DEBUG_YACC_FLAGS = --debug --yacc# --report=all --graph

LEX_FLAGS = --warn
RELEASE_LEX_FLAGS =
DEBUG_LEX_FLAGS = --debug --perf-report --perf-report --nodefault --posix# --lex-compat --verbose

CFLAGS = -Wall -Wextra -pedantic -I$(SRC) -I$(BUILD) -Wno-alloc-size-larger-than
RELEASE_CFLAGS = -Ofast
DEBUG_CFLAGS = -std=c99 -DDEBUG -D_POSIX_VERSION=200112L -D_POSIX_C_SOURCE=200112L -g -O0

all: setup release

.PHONY: setup
setup:
	mkdir -p $(BUILD)

.PHONY: release-full
release-full: clean setup release check

.PHONY: release
release: LEX_FLAGS += $(RELEASE_LEX_FLAGS)
release: YACC_FLAGS += $(RELEASE_YACC_FLAGS)
release: CFLAGS += $(RELEASE_CFLAGS)
release: $(EXE)

.PHONY: debug-full
debug-full: clean setup debug check

.PHONY: debug
debug: LEX_FLAGS += $(DEBUG_LEX_FLAGS)
debug: YACC_FLAGS += $(DEBUG_YACC_FLAGS)
debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(EXE)

$(BUILD)/parser.o: $(SRC)/parser.y
	$(YACC) $(YACC_FLAGS) -o $(BUILD)/parser.c $(SRC)/parser.y
	$(CC) $(CFLAGS) -c $(BUILD)/parser.c -o $(BUILD)/parser.o

$(BUILD)/lexer.o: $(SRC)/lexer.l
	$(LEX) $(LEX_FLAGS) -o $(BUILD)/lexer.c $(SRC)/lexer.l
	$(CC) $(CFLAGS) -c $(BUILD)/lexer.c -o $(BUILD)/lexer.o

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(addprefix $(BUILD)/, $(OBJS)) $(BUILD)/texalc.o
	$(CC) $(CFLAGS) $^ -o $(EXE)

$(EXE)-test: $(addprefix $(BUILD)/, $(OBJS)) $(BUILD)/test.o
	$(CC) $(CFLAGS) $^ -o $(EXE)-test

.PHONY: check
check: test

.PHONY: test
test: $(EXE)-test
	./test.sh

.PHONY: clean
clean:
	rm -fr $(BUILD)
	rm -fr $(EXE)

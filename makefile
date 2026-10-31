# Makefile for MM-11
#
#   make            optimized build       -> ./mm_11
#   make debug      ASan + UBSan build    -> ./mm_11-debug
#   make test       golden-file tests against ./mm_11
#   make test-debug the same tests under the sanitizers
#   make docs       regenerate COMMANDS.md from the dispatch table
#   make install    copy to $(PREFIX)/bin

CC       ?= cc
CSTD      = -std=c17
WARN      = -Wall -Wextra -Werror -Wshadow -Wformat=2
CPPFLAGS += -D_POSIX_C_SOURCE=200809L -MMD -MP
CFLAGS   ?= -O2 -g
LDLIBS    = -lreadline -lm
PREFIX   ?= /usr/local

TARGET   ?= mm_11
BUILD    ?= build

SRC = main.c interp.c ops.c words.c state.c stack.c registers.c my_math.c \
      my_statistics.c stack_printing.c help_functions.c tab_complete.c    \
      tokenizer.c util_func.c
OBJ = $(SRC:%.c=$(BUILD)/%.o)
DEP = $(OBJ:.o=.d)

# macOS ships libedit as "readline"; prefer Homebrew's GNU readline if present.
ifeq ($(shell uname -s),Darwin)
  RL_PREFIX ?= $(shell brew --prefix readline 2>/dev/null)
  ifneq ($(RL_PREFIX),)
    CPPFLAGS += -I$(RL_PREFIX)/include
    LDFLAGS  += -L$(RL_PREFIX)/lib
  else
    LDLIBS = -ledit -lm
  endif
endif

SAN = -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all

.PHONY: all debug test test-debug docs install clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CSTD) $(WARN) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

debug:
	$(MAKE) TARGET=mm_11-debug BUILD=build-debug CFLAGS="$(SAN)" LDFLAGS="$(LDFLAGS) -fsanitize=address,undefined"

test: $(TARGET)
	./tests/run.sh ./$(TARGET)

test-debug: debug
	ASAN_OPTIONS=detect_leaks=1 ./tests/run.sh ./mm_11-debug

docs: $(TARGET)
	./$(TARGET) --commands > COMMANDS.md

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -rf build build-debug mm_11 mm_11-debug

-include $(DEP)

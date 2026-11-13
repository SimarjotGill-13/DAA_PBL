CC ?= gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDLIBS = -lm
SRC = $(wildcard src/*.c)
BIN = order_optimizer

all: $(BIN)

$(BIN): $(SRC) include/common.h
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDLIBS)

test: $(BIN)
	./$(BIN) --test

clean:
	rm -f $(BIN) optimizer_test_dataset.tmp

.PHONY: all test clean

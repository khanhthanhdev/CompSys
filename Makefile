CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
SRC     = src/main.c src/team.c src/match.c src/queue.c src/ranking.c src/file_io.c
OBJ     = $(SRC:.c=.o)
TARGET  = tournament
TEST_CORE_SRC = tests/test_core.c src/team.c src/match.c src/queue.c src/ranking.c src/file_io.c
TEST_CORE     = tests/test_core

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_CORE): $(TEST_CORE_SRC)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TARGET) $(TEST_CORE)
	./$(TEST_CORE)
	sh tests/run_integration.sh $(abspath $(TARGET)) $(CURDIR)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

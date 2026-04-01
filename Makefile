CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
SRC     = src/main.c src/team.c src/match.c src/queue.c src/ranking.c src/file_io.c
OBJ     = $(SRC:.c=.o)
TARGET  = tournament

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

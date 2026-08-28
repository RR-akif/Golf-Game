CC = gcc
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
TARGET = game

CFLAGS = -Wall -Wextra -std=c99 $(shell pkg-config --cflags raylib)
LIBS = $(shell pkg-config --libs raylib) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: run clean
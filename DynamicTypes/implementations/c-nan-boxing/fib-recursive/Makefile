CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99
TARGET = fib_nanbox
SOURCE = fib.c

all: $(TARGET)

$(TARGET): $(SOURCE) ../nanbox.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

benchmark: $(TARGET)
	./$(TARGET)

.PHONY: all clean benchmark
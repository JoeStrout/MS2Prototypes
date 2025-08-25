CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99
TARGET = fib_nanbox
SOURCES = fib.c ../gc.c ../nanbox_strings.c ../unicodeUtil.c

all: $(TARGET)

$(TARGET): $(SOURCES) ../nanbox_gc.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

benchmark: $(TARGET)
	./$(TARGET)

.PHONY: all clean benchmark
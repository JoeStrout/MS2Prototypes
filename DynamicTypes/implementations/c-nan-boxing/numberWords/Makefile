CC = gcc
CFLAGS = -O3 -std=c99 -Wall -Wextra -I..
TARGET = numberWords_nanbox
SOURCES = numberWords.c

$(TARGET): $(SOURCES) ../nanbox.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

benchmark: $(TARGET)
	./$(TARGET)

.PHONY: clean benchmark
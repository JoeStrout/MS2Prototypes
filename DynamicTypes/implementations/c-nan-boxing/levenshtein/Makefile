CC = gcc
CFLAGS = -O3 -std=c99 -Wall -Wextra -I..
TARGET = levenshtein_nanbox
SOURCES = levenshtein.c

$(TARGET): $(SOURCES) ../nanbox.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

benchmark: $(TARGET)
	./$(TARGET)

.PHONY: clean benchmark
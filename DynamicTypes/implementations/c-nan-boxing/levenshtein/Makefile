CC = gcc
CFLAGS = -O3 -std=c99 -Wall -Wextra -I..
TARGET = levenshtein_nanbox
SOURCES = levenshtein.c ../gc.c

$(TARGET): $(SOURCES) ../nanbox_gc.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

benchmark: $(TARGET)
	./$(TARGET)

.PHONY: clean benchmark
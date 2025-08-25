CC = gcc
CFLAGS = -O3 -std=c99 -Wall -Wextra -I..
TARGET = numberWords_nanbox
SOURCES = numberWords.c ../gc.c ../unicodeUtil.c ../nanbox_strings.c

$(TARGET): $(SOURCES) ../nanbox_gc.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

benchmark: $(TARGET)
	./$(TARGET)

.PHONY: clean benchmark
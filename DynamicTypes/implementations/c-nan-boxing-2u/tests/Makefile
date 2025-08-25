CC = gcc
CFLAGS = -O3 -std=c99 -Wall -Wextra -I..

all: test_strings test_lists

test_strings: test_strings.c
	$(CC) $(CFLAGS) -o test_strings test_strings.c

test_lists: test_lists.c
	$(CC) $(CFLAGS) -o test_lists test_lists.c

clean:
	rm -f test_strings test_lists

.PHONY: all clean
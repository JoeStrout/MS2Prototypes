# Makefile for c-nan-boxing-2u with Unicode support

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -g

# Object files
OBJS = gc.o unicodeUtil.o nanbox_strings.o

# Default target
all: test_unicode test_strings

# Unicode test executable
test_unicode: test_unicode.o $(OBJS)
	$(CC) $(CFLAGS) -o test_unicode test_unicode.o $(OBJS)

# Simple Unicode test executable
test_unicode_simple: test_unicode_simple.o $(OBJS)
	$(CC) $(CFLAGS) -o test_unicode_simple test_unicode_simple.o $(OBJS)

# Test concatenation only
test_concat_only: test_concat_only.o $(OBJS)
	$(CC) $(CFLAGS) -o test_concat_only test_concat_only.o $(OBJS)

# Test split only
test_split_only: test_split_only.o $(OBJS)
	$(CC) $(CFLAGS) -o test_split_only test_split_only.o $(OBJS)

# Test length only
test_length_only: test_length_only.o $(OBJS)
	$(CC) $(CFLAGS) -o test_length_only test_length_only.o $(OBJS)

# Debug concat issue
debug_concat_issue: debug_concat_issue.o $(OBJS)
	$(CC) $(CFLAGS) -o debug_concat_issue debug_concat_issue.o $(OBJS)

# Original string test executable
test_strings: tests/test_strings.o $(OBJS)
	$(CC) $(CFLAGS) -o test_strings tests/test_strings.o $(OBJS)

# List test executable
test_lists: tests/test_lists.o $(OBJS)
	$(CC) $(CFLAGS) -o test_lists tests/test_lists.o $(OBJS)

# Object files
test_unicode.o: test_unicode.c nanbox.h nanbox_gc.h unicodeUtil.h
	$(CC) $(CFLAGS) -c test_unicode.c

test_unicode_simple.o: test_unicode_simple.c nanbox.h nanbox_gc.h unicodeUtil.h
	$(CC) $(CFLAGS) -c test_unicode_simple.c

tests/test_strings.o: tests/test_strings.c nanbox.h nanbox_gc.h unicodeUtil.h
	$(CC) $(CFLAGS) -c tests/test_strings.c -o tests/test_strings.o

tests/test_lists.o: tests/test_lists.c nanbox.h nanbox_gc.h unicodeUtil.h
	$(CC) $(CFLAGS) -c tests/test_lists.c -o tests/test_lists.o

gc.o: gc.c nanbox_gc.h nanbox.h
	$(CC) $(CFLAGS) -c gc.c

unicodeUtil.o: unicodeUtil.c unicodeUtil.h
	$(CC) $(CFLAGS) -c unicodeUtil.c

nanbox_strings.o: nanbox_strings.c nanbox.h nanbox_gc.h unicodeUtil.h
	$(CC) $(CFLAGS) -c nanbox_strings.c

# Test targets
test: test_unicode test_strings
	@echo "Running Unicode tests..."
	./test_unicode
	@echo ""
	@echo "Running original string tests..."
	./test_strings

# Clean up
clean:
	rm -f *.o tests/*.o test_unicode test_unicode_simple test_strings

.PHONY: all test clean
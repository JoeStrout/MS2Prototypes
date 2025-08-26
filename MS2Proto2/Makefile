# MS2Proto2 - Modular NaN-Boxing Dynamic Type System
# Build system for the core library and tests/benchmarks

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -g

# Directories
SRCDIR = src
INCDIR = include
TESTDIR = tests
BENCHDIR = benchmarks
EXAMPLEDIR = examples

# Core library sources
CORE_SOURCES = $(SRCDIR)/unicode.c $(SRCDIR)/nanbox.c $(SRCDIR)/gc.c $(SRCDIR)/strings.c $(SRCDIR)/lists.c
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)

# Library target
LIBNAME = libms2proto2.a

# Test targets
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_TARGETS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(TESTDIR)/%)

# Benchmark targets
BENCH_SOURCES = $(wildcard $(BENCHDIR)/*.c)
BENCH_TARGETS = $(BENCH_SOURCES:$(BENCHDIR)/%.c=$(BENCHDIR)/%)

# Default target
all: $(LIBNAME)

# Core library
$(LIBNAME): $(CORE_OBJECTS)
	ar rcs $@ $^

# Object files
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Tests
tests: $(TEST_TARGETS)

$(TESTDIR)/%: $(TESTDIR)/%.c $(LIBNAME)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< -L. -lms2proto2

# Benchmarks  
benchmarks: $(BENCH_TARGETS)

$(BENCHDIR)/%: $(BENCHDIR)/%.c $(LIBNAME)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< -L. -lms2proto2

# Examples
examples: $(EXAMPLE_TARGETS)

$(EXAMPLEDIR)/%: $(EXAMPLEDIR)/%.c $(LIBNAME)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< -L. -lms2proto2

# Clean
clean:
	rm -f $(CORE_OBJECTS) $(LIBNAME) $(TEST_TARGETS) $(BENCH_TARGETS)

# Install headers (for external projects)
install-headers:
	mkdir -p /usr/local/include/ms2proto2
	cp $(INCDIR)/*.h /usr/local/include/ms2proto2/

.PHONY: all tests benchmarks examples clean install-headers
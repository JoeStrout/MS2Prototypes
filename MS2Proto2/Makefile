# MS2Proto2 - Modular NaN-Boxing Dynamic Type System
# Build system for the core library and tests/benchmarks

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -g -D_DEFAULT_SOURCE

# Directories
SRCDIR = src
INCDIR = include
TESTDIR = tests
BENCHDIR = benchmarks
EXAMPLEDIR = examples

# Core library sources
CORE_SOURCES = $(SRCDIR)/types/unicode.c $(SRCDIR)/types/nanbox.c $(SRCDIR)/types/gc.c $(SRCDIR)/types/strings.c $(SRCDIR)/types/lists.c
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)

# VM sources
VM_SOURCES = $(SRCDIR)/vm/vm.c
VM_OBJECTS = $(VM_SOURCES:.c=.o)

# Assembler sources
ASM_SOURCES = $(SRCDIR)/assembler/assembler.c
ASM_OBJECTS = $(ASM_SOURCES:.c=.o)

# All library sources
ALL_SOURCES = $(CORE_SOURCES) $(VM_SOURCES) $(ASM_SOURCES)
ALL_OBJECTS = $(CORE_OBJECTS) $(VM_OBJECTS) $(ASM_OBJECTS)

# Library target
LIBNAME = libms2proto2.a

# Test targets
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_TARGETS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(TESTDIR)/%)

# Benchmark targets
BENCH_SOURCES = $(wildcard $(BENCHDIR)/*.c)
BENCH_TARGETS = $(BENCH_SOURCES:$(BENCHDIR)/%.c=$(BENCHDIR)/%)

# Default target
all: $(LIBNAME) ms2vm ms2asm

# Core library
$(LIBNAME): $(ALL_OBJECTS)
	ar rcs $@ $^

# Main VM executable
ms2vm: $(SRCDIR)/main.c $(LIBNAME)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< -L. -lms2proto2

# Assembler executable
ms2asm: $(SRCDIR)/assembler/main.c $(LIBNAME)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $@ $< -L. -lms2proto2

# Object files
$(SRCDIR)/types/%.o: $(SRCDIR)/types/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(SRCDIR)/vm/%.o: $(SRCDIR)/vm/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(SRCDIR)/assembler/%.o: $(SRCDIR)/assembler/%.c
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
	rm -f $(ALL_OBJECTS) $(LIBNAME) $(TEST_TARGETS) $(BENCH_TARGETS) ms2vm ms2asm

# Install headers (for external projects)
install-headers:
	mkdir -p /usr/local/include/ms2proto2/types
	mkdir -p /usr/local/include/ms2proto2/vm
	mkdir -p /usr/local/include/ms2proto2/assembler
	cp $(INCDIR)/types/*.h /usr/local/include/ms2proto2/types/
	cp $(INCDIR)/vm/*.h /usr/local/include/ms2proto2/vm/ 2>/dev/null || true
	cp $(INCDIR)/assembler/*.h /usr/local/include/ms2proto2/assembler/ 2>/dev/null || true

.PHONY: all tests benchmarks examples clean install-headers
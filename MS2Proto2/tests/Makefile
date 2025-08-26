# Tests Makefile for MS2Proto2
# Builds and runs individual or all test suites

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -g
INCLUDES = -I../include
LIBDIR = -L..
LIBS = -lms2proto2

# Library dependency
LIBMS2 = ../libms2proto2.a

# Test source files
TEST_SOURCES = $(wildcard test_*.c)
TEST_TARGETS = $(TEST_SOURCES:.c=)

# Default target - build all tests
all: $(TEST_TARGETS)

# Pattern rule to build individual tests
test_%: test_%.c $(LIBMS2)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBDIR) $(LIBS)

# Build the library if it doesn't exist
$(LIBMS2):
	$(MAKE) -C .. libms2proto2.a

# Run all tests
run-all: all
	@echo "Running all tests..."
	@for test in $(TEST_TARGETS); do \
		echo ""; \
		echo "=== Running $$test ==="; \
		./$$test; \
	done
	@echo ""
	@echo "All tests completed!"

# Run individual tests
run-%: %
	@echo "=== Running $< ==="
	./$<

# Individual test targets for convenience
nanbox: test_nanbox
unicode: test_unicode
gc: test_gc
gc-simple: test_gc_simple
lists: test_lists
lists-capacity: test_lists_capacity

# Run targets
run-nanbox: run-test_nanbox
run-unicode: run-test_unicode
run-gc: run-test_gc
run-gc-simple: run-test_gc_simple
run-lists: run-test_lists
run-lists-capacity: run-test_lists_capacity

# Clean targets
clean:
	rm -f $(TEST_TARGETS)
	rm -rf *.dSYM

# Force rebuild library and tests
rebuild: clean
	$(MAKE) -C .. clean
	$(MAKE) -C .. libms2proto2.a
	$(MAKE) all

# Help target
help:
	@echo "MS2Proto2 Test Suite"
	@echo "==================="
	@echo ""
	@echo "Build targets:"
	@echo "  all              - Build all test executables"
	@echo "  nanbox           - Build nanbox core tests"
	@echo "  unicode          - Build unicode tests"  
	@echo "  gc               - Build comprehensive GC tests"
	@echo "  gc-simple        - Build simple GC tests"
	@echo "  lists            - Build comprehensive lists tests"
	@echo "  lists-capacity   - Build lists capacity expansion tests"
	@echo ""
	@echo "Run targets:"
	@echo "  run-all          - Run all tests"
	@echo "  run-nanbox       - Run nanbox core tests"
	@echo "  run-unicode      - Run unicode tests"
	@echo "  run-gc           - Run comprehensive GC tests"
	@echo "  run-gc-simple    - Run simple GC tests"
	@echo "  run-lists        - Run comprehensive lists tests"
	@echo "  run-lists-capacity - Run lists capacity expansion tests"
	@echo ""
	@echo "Utility targets:"
	@echo "  clean            - Remove test executables"
	@echo "  rebuild          - Clean, rebuild library, and rebuild tests"
	@echo "  help             - Show this help"

# Ensure targets don't conflict with files
.PHONY: all run-all clean rebuild help nanbox unicode gc gc-simple lists lists-capacity
.PHONY: run-nanbox run-unicode run-gc run-gc-simple run-lists run-lists-capacity
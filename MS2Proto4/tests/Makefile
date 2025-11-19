# Makefile for CS_String unit tests

CXX = g++
CC = gcc
CXXFLAGS = -std=gnu++11 -Wall -Wextra -g
CFLAGS = -Wall -Wextra -g
INCLUDES = -I../cpp/core

CORE_DIR = ../cpp/core

# C++ source files
CXX_SOURCES = \
	$(CORE_DIR)/CS_String.cpp \
	$(CORE_DIR)/CS_List.cpp \
	test_CS_String.cpp

# C source files
C_SOURCES = \
	$(CORE_DIR)/StringStorage.c \
	$(CORE_DIR)/unicodeUtil.c \
	$(CORE_DIR)/hashing.c

# Object files
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)
ALL_OBJECTS = $(CXX_OBJECTS) $(C_OBJECTS)

# Output binary
TARGET = test_CS_String

# Default target
all: $(TARGET)

$(TARGET): $(CXX_OBJECTS) $(C_OBJECTS)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) $(ALL_OBJECTS) -o $(TARGET)
	@echo "Build complete: ./$(TARGET)"

# Compile C++ files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile C files
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run tests
test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET)

# Clean
clean:
	rm -f $(TARGET) $(ALL_OBJECTS)
	@echo "Clean complete"

.PHONY: all test clean

#!/bin/bash
# Build script for CS_Dictionary unit tests

set -e  # Exit on error

echo "Building CS_Dictionary unit tests..."

# Compiler settings
CXX=g++
CC=gcc
CXXFLAGS="-std=gnu++11 -Wall -Wextra -g"
CFLAGS="-Wall -Wextra -g"
INCLUDES="-I../cpp/core"

# Core directory
CORE_DIR="../cpp/core"

# Output binary
OUTPUT="test_CS_Dictionary"

# Compile C sources
echo "Compiling C sources..."
$CC $CFLAGS $INCLUDES -c $CORE_DIR/StringStorage.c -o StringStorage.o
$CC $CFLAGS $INCLUDES -c $CORE_DIR/unicodeUtil.c -o unicodeUtil.o
$CC $CFLAGS $INCLUDES -c $CORE_DIR/hashing.c -o hashing.o

# Compile C++ sources
echo "Compiling C++ sources..."
$CXX $CXXFLAGS $INCLUDES -c $CORE_DIR/CS_String.cpp -o CS_String.o
$CXX $CXXFLAGS $INCLUDES -c test_CS_Dictionary.cpp -o test_CS_Dictionary.o

# Link
echo "Linking..."
$CXX $CXXFLAGS StringStorage.o unicodeUtil.o hashing.o CS_String.o test_CS_Dictionary.o -o $OUTPUT

echo "Build complete: ./$OUTPUT"
echo ""
echo "Run with: ./$OUTPUT"

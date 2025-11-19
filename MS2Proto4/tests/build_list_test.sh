#!/bin/bash
# Build script for CS_List unit tests

set -e  # Exit on error

echo "Building CS_List unit tests..."

# Compiler settings
CXX=g++
CXXFLAGS="-std=gnu++11 -Wall -Wextra -g"
INCLUDES="-I../cpp/core"

# Output binary
OUTPUT="test_CS_List"

# Compile (CS_List is header-only, so just compile the test file)
echo "Compiling..."
$CXX $CXXFLAGS $INCLUDES test_CS_List.cpp -o $OUTPUT

echo "Build complete: ./$OUTPUT"
echo ""
echo "Run with: ./$OUTPUT"

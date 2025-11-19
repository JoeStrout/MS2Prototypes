#!/bin/bash
# Build script for CS_String unit tests

set -e  # Exit on error

echo "Building CS_String unit tests..."

# Compiler settings
CXX=g++
CXXFLAGS="-std=gnu++11 -Wall -Wextra -g"
INCLUDES="-I../cpp/core"

# Source files
CORE_DIR="../cpp/core"
SOURCES=(
    "$CORE_DIR/CS_String.cpp"
    "$CORE_DIR/CS_List.cpp"
    "$CORE_DIR/StringStorage.c"
    "$CORE_DIR/unicodeUtil.c"
    "$CORE_DIR/hashing.c"
    "test_CS_String.cpp"
)

# Output binary
OUTPUT="test_CS_String"

# Compile
echo "Compiling..."
$CXX $CXXFLAGS $INCLUDES "${SOURCES[@]}" -o $OUTPUT

echo "Build complete: ./$OUTPUT"
echo ""
echo "Run with: ./$OUTPUT"

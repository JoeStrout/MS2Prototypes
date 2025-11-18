#!/bin/bash
# MS2Proto4 build orchestration script

set -e  # Exit on any error

PROJECT_ROOT="$(dirname "$0")/.."
cd "$PROJECT_ROOT"

echo "=== MS2Proto4 Build Script ==="
echo "Project root: $(pwd)"

# Parse command line arguments
TARGET="${1:-all}"

case "$TARGET" in
    "setup")
        echo "Setting up development environment..."
        mkdir -p build/cpp
        mkdir -p generated
        echo "Setup complete."
        ;;

    "transpile")
        echo "Transpiling C# to C++..."
        mkdir -p generated

        # Find all .cs files in the cs directory, excluding build artifacts
        cs_files=$(find cs -name "*.cs" -type f -not -path "*/obj/*" -not -path "*/bin/*")

        if [ -z "$cs_files" ]; then
            echo "No .cs files found in cs/ directory."
            exit 1
        fi

        echo "Found C# files:"
        echo "$cs_files"

        # Transpile all .cs files in a single call
        echo "Transpiling all files..."
        miniscript tools/transpile.ms $cs_files
        if [ $? -ne 0 ]; then
            echo "Error: Failed to transpile C# files"
            exit 1
        fi

        echo "Transpilation complete."
        ;;

    "cpp")
        echo "Building C++ version..."
        if [ ! -d "generated" ] || [ -z "$(ls -A generated 2>/dev/null)" ]; then
            echo "Transpiled C++ files not found. Run 'transpile' first."
            exit 1
        fi
        if [ ! -f "cpp/Makefile" ]; then
            echo "cpp/Makefile not found. Please create one."
            exit 1
        fi
        make -C cpp
        echo "C++ build complete."
        ;;

    "all")
        echo "Building all targets..."
        $0 transpile
        $0 cpp
        echo "All builds complete."
        ;;

    "clean")
        echo "Cleaning all build artifacts..."
        rm -rf build/cpp/*
        rm -rf generated/*.g.h generated/*.g.cpp
        if [ -f cpp/Makefile ]; then
            make -C cpp clean
        fi
        echo "Clean complete."
        ;;

    *)
        echo "Usage: $0 {setup|transpile|cpp|all|clean}"
        echo "  setup     - Set up development environment"
        echo "  transpile - Transpile C# to C++"
        echo "  cpp       - Build C++ version only"
        echo "  all       - Build everything"
        echo "  clean     - Clean build artifacts"
        exit 1
        ;;
esac

echo "Build script completed successfully."

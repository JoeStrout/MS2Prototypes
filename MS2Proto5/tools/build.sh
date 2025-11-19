#!/bin/bash
# MS2Proto5 build orchestration script

set -e  # Exit on any error

PROJECT_ROOT="$(dirname "$0")/.."
cd "$PROJECT_ROOT"

echo "=== MS2Proto5 Build Script ==="
echo "Project root: $(pwd)"

# Parse command line arguments
TARGET="${1:-all}"

case "$TARGET" in
    "setup")
        echo "Setting up development environment..."
        mkdir -p build/cpp
        mkdir -p build/cs
        mkdir -p generated
        echo "Setup complete."
        ;;

    "cs")
        echo "Building C# version..."
        if [ ! -d "cs" ]; then
            echo "cs/ directory not found."
            exit 1
        fi

        # Check if dotnet is available
        if command -v dotnet &> /dev/null; then
            echo "Using dotnet to build..."
            cd cs
            dotnet build -c Release -o ../build/cs
            cd ..
            echo "C# build complete. Executable: build/cs/MS2Proto5"
        elif command -v mcs &> /dev/null && command -v mono &> /dev/null; then
            echo "Using mcs/mono to build..."
            mkdir -p build/cs
            mcs cs/*.cs -out:build/cs/MS2Proto5.exe
            echo "C# build complete. Run with: mono build/cs/MS2Proto5.exe"
        else
            echo "Error: Neither dotnet nor mcs/mono found."
            echo "Please install .NET SDK or Mono to build C# code."
            exit 1
        fi
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
        $0 cs
        $0 transpile
        $0 cpp
        echo "All builds complete."
        ;;

    "clean")
        echo "Cleaning all build artifacts..."
        rm -rf build/cpp/*
        rm -rf build/cs/*
        rm -rf generated/*.g.h generated/*.g.cpp
        if [ -f cpp/Makefile ]; then
            make -C cpp clean
        fi
        if [ -d cs/bin ]; then
            rm -rf cs/bin
        fi
        if [ -d cs/obj ]; then
            rm -rf cs/obj
        fi
        echo "Clean complete."
        ;;

    *)
        echo "Usage: $0 {setup|cs|transpile|cpp|all|clean}"
        echo "  setup     - Set up development environment"
        echo "  cs        - Build C# version"
        echo "  transpile - Transpile C# to C++"
        echo "  cpp       - Build C++ version only"
        echo "  all       - Build everything (C#, transpile, C++)"
        echo "  clean     - Clean build artifacts"
        exit 1
        ;;
esac

echo "Build script completed successfully."

#!/bin/bash
# MS2Proto3 build orchestration script

set -e  # Exit on any error

PROJECT_ROOT="$(dirname "$0")/.."
cd "$PROJECT_ROOT"

echo "=== MS2Proto3 Build Script ==="
echo "Project root: $(pwd)"

# Parse command line arguments
TARGET="${1:-all}"

case "$TARGET" in
    "setup")
        echo "Setting up development environment..."
        mkdir -p build/{cs,cpp,temp}
        mkdir -p generated
        echo "Setup complete."
        ;;
    
    "cs")
        echo "Building C# version..."
        cd cs
        dotnet build -o ../build/cs/
        echo "C# build complete."
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
        
        # Transpile each .cs file
        for cs_file in $cs_files; do
            echo "Transpiling $cs_file..."
            miniscript tools/transpile.ms "$cs_file"
            if [ $? -ne 0 ]; then
                echo "Error: Failed to transpile $cs_file"
                exit 1
            fi
        done
        
        echo "Transpilation complete."
        ;;
    
    "cpp")
        echo "Building C++ version..."
        if [ ! -f "generated/Program.g.cpp" ]; then
            echo "No transpiled C++ files found. Run 'transpile' first."
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
        rm -rf build/cs/* build/cpp/* build/temp/*
        rm -rf generated/*.g.h generated/*.g.cpp
        cd cs && dotnet clean
        if [ -f cpp/Makefile ]; then
            make -C cpp clean
        fi
        echo "Clean complete."
        ;;
    
    "test")
        echo "Running tests..."
        echo "Testing C# version:"
        cd build/cs && echo "test input" | ./MS2Proto3
        cd ../..
        echo "Testing C++ version:"
        cd build/cpp && echo "test input" | ./MS2Proto3
        cd ../..
        ;;
    
    *)
        echo "Usage: $0 {setup|cs|transpile|cpp|all|clean|test}"
        echo "  setup     - Set up development environment"
        echo "  cs        - Build C# version only"
        echo "  transpile - Transpile C# to C++"
        echo "  cpp       - Build C++ version only"
        echo "  all       - Build everything"
        echo "  clean     - Clean build artifacts"
        echo "  test      - Build and test both versions"
        exit 1
        ;;
esac

echo "Build script completed successfully."
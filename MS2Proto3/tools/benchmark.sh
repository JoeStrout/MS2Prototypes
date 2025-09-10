#!/bin/bash

# MS2Proto3 Benchmarking Script
# Runs performance benchmarks on both C# and C++ builds
# Verifies correctness of results

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m' # No Color

echo -e "${BOLD}=== MS2Proto3 Benchmark Suite ===${NC}"
echo "Project root: $PROJECT_ROOT"
echo ""

# Expected results for verification (allow for different scientific notation formats)
verify_result() {
    local actual="$1"
    local expected="$2"
    local name="$3"
    
    if [[ "$name" == "Iterative Factorial" ]]; then
        # For factorial, accept both full precision and scientific notation
        if [[ "$actual" == "2.43290200817664E+18" || "$actual" == "2.4329e+18" ]]; then
            return 0
        fi
    elif [[ "$actual" == "$expected" ]]; then
        return 0
    fi
    return 1
}

EXPECTED_ITER_FIB="832040"                 # fib(30) * 500000 iterations  
EXPECTED_RECUR_FIB="3524578"              # fib(33)

# Benchmark definitions  
BENCHMARKS=(
    "factorial_iterative.msa:Iterative Factorial:unused"
    "iter_fib.msa:Iterative Fibonacci:$EXPECTED_ITER_FIB" 
    "recur_fib.msa:Recursive Fibonacci:$EXPECTED_RECUR_FIB"
)

# For quick testing, uncomment the line below to run only one benchmark:
# BENCHMARKS=("factorial_iterative.msa:Iterative Factorial:unused")

# Function to run a single benchmark and return timing
run_benchmark() {
    local benchmark_file="$1"
    local benchmark_name="$2" 
    local expected_result="$3"
    local build_type="$4"
    local executable="$5"
    
    # Run benchmark and capture timing and result
    if [[ "$build_type" == "C#" ]]; then
        TIME_OUTPUT=$(time (dotnet "$executable" "examples/$benchmark_file" 2>/dev/null) 2>&1)
        RESULT=$(dotnet "$executable" "examples/$benchmark_file" 2>/dev/null | grep -o "Result in r0:" -A1 | tail -1 | sed 's/\x1b\[[0-9;]*m//g')
    else
        TIME_OUTPUT=$(time ("$executable" "examples/$benchmark_file" 2>/dev/null) 2>&1)
        RESULT=$("$executable" "examples/$benchmark_file" 2>/dev/null | grep -o "Result in r0:" -A1 | tail -1 | sed 's/\x1b\[[0-9;]*m//g')
    fi
    
    # Extract timing information (handle both formats: "real 0m3.072s" or "3.072 total")
    if echo "$TIME_OUTPUT" | grep -q "real"; then
        REAL_TIME=$(echo "$TIME_OUTPUT" | grep "real" | awk '{print $2}')
    else
        REAL_TIME=$(echo "$TIME_OUTPUT" | grep "total" | awk '{print $(NF-1)}')
    fi
    
    # Verify result - exit immediately if wrong
    if verify_result "$RESULT" "$expected_result" "$benchmark_name"; then
        echo "    ✓ $RESULT (${REAL_TIME})" >&2
    else
        echo "    ✗ FAILED: $RESULT (expected: $expected_result)" >&2
        echo "Benchmark failed - exiting." >&2
        exit 1
    fi
    
    # Convert timing to seconds and return just the number
    if [[ "$REAL_TIME" =~ ^([0-9]+)m([0-9.]+)s$ ]]; then
        # Format: 0m3.072s  
        minutes="${BASH_REMATCH[1]}"
        seconds="${BASH_REMATCH[2]}"
        echo "scale=3; $minutes * 60 + $seconds" | bc 2>/dev/null || echo "$seconds"
    else
        # Format: 3.072s or just 3.072
        echo "$REAL_TIME" | sed 's/s$//'
    fi
}

# Arrays to store results
declare -a CS_TIMES
declare -a CPP_GOTO_TIMES
declare -a CPP_SWITCH_TIMES

echo -e "${BOLD}=== Benchmark Results ===${NC}"
echo ""

# Build and run all C# benchmarks
echo -e "${BOLD}Building C# version...${NC}"
tools/build.sh cs > /dev/null
echo -e "${BOLD}Running C# benchmarks...${NC}"
for i in "${!BENCHMARKS[@]}"; do
    benchmark_def="${BENCHMARKS[i]}"
    IFS=':' read -r file name expected <<< "$benchmark_def"
    
    echo -e "${BLUE}  $name...${NC}"
    cs_time=$(run_benchmark "$file" "$name" "$expected" "C#" "build/cs/MS2Proto3.dll")
    CS_TIMES+=("$cs_time")
done
echo ""

# Build and run all C++ (switch-based) benchmarks
echo -e "${BOLD}Building C++ version (switch-based)...${NC}"
rm -f build/cpp/obj/* build/cpp/MS2Proto3 2>/dev/null
if ! tools/build.sh cpp off; then
    echo -e "${RED}C++ (switch-based) build failed!${NC}"
    exit 1
fi
echo -e "${BOLD}Running C++ (switch-based) benchmarks...${NC}"
for i in "${!BENCHMARKS[@]}"; do
    benchmark_def="${BENCHMARKS[i]}"
    IFS=':' read -r file name expected <<< "$benchmark_def"
    
    echo -e "${BLUE}  $name...${NC}"
    cpp_switch_time=$(run_benchmark "$file" "$name" "$expected" "C++ (switch-based)" "build/cpp/MS2Proto3")
    CPP_SWITCH_TIMES+=("$cpp_switch_time")
done
echo ""

# Build and run all C++ (computed-goto) benchmarks
echo -e "${BOLD}Building C++ version (computed-goto)...${NC}"
rm -f build/cpp/obj/* build/cpp/MS2Proto3 2>/dev/null
if ! tools/build.sh cpp on; then
    echo -e "${RED}C++ (computed-goto) build failed!${NC}"
    exit 1
fi
echo -e "${BOLD}Running C++ (computed-goto) benchmarks...${NC}"
for i in "${!BENCHMARKS[@]}"; do
    benchmark_def="${BENCHMARKS[i]}"
    IFS=':' read -r file name expected <<< "$benchmark_def"
    
    echo -e "${BLUE}  $name...${NC}"
    cpp_goto_time=$(run_benchmark "$file" "$name" "$expected" "C++ (computed-goto)" "build/cpp/MS2Proto3")
    CPP_GOTO_TIMES+=("$cpp_goto_time")
done
echo ""

# Clean markdown-style summary table
echo -e "${BOLD}=== Performance Summary ===${NC}"
echo ""
echo "| Benchmark               | C#        | C++ (switch) | C++ (goto) |"
echo "|-------------------------|-----------|--------------|------------|"

for i in "${!BENCHMARKS[@]}"; do
    IFS=':' read -r file name expected <<< "${BENCHMARKS[i]}"
    cs_time="${CS_TIMES[i]}"
    cpp_goto_time="${CPP_GOTO_TIMES[i]}"
    cpp_switch_time="${CPP_SWITCH_TIMES[i]}"
    
    printf "| %-23s | %-9s | %-12s | %-10s |\n" "$name" "${cs_time}s" "${cpp_switch_time}s" "${cpp_goto_time}s"
done

echo ""
echo -e "${GREEN}Benchmark suite completed!${NC}"
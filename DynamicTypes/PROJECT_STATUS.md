# Dynamic Types Benchmark Project - Status Summary

## Project Overview
This project implements various dynamic typing systems and benchmarks them against each other. The focus is on comparing performance and implementation complexity across different approaches to dynamic typing in systems programming languages.

## Repository Structure
```
DynamicTypes/
├── benchmarks/              # Original MiniScript benchmark implementations
│   ├── fib-recursive.ms     # Recursive Fibonacci 
│   ├── levenshtein.ms       # Edit distance algorithm
│   └── numberWords.ms       # Number-to-words bidirectional conversion
└── implementations/         # C/C++ implementations using different type systems
    ├── c-nan-boxing/        # NaN boxing technique for dynamic types
    │   ├── fib-recursive/   # Completed
    │   ├── levenshtein/     # Completed
    │   ├── numberWords/     # Completed  
    │   ├── tests/           # Comprehensive unit tests
    │   └── nanbox.h         # Shared NaN boxing implementation
    ├── c-nan-boxing-2/      # NaN boxing with tiny string optimization  
    │   ├── fib-recursive/   # Completed
    │   ├── levenshtein/     # Completed
    │   ├── numberWords/     # Completed
    │   ├── tests/           # GC and tiny string test suites
    │   ├── nanbox.h         # Enhanced with tiny string support
    │   └── nanbox_gc.h      # Shadow stack garbage collector
    └── cpp-classic/         # MiniScript 1.x C++ type system
        ├── fib-recursive/   # Completed
        ├── levenshtein/     # Completed
        └── numberWords/     # Completed
```

## Implemented Benchmarks

### 1. Recursive Fibonacci (`fib-recursive`)
**Purpose**: Tests function call overhead and integer arithmetic with dynamic types  
**Algorithm**: Classic recursive fib(n) = fib(n-1) + fib(n-2)  
**Test case**: n=30, expected result = 832040  

**Implementations completed:**
- ✅ **C NaN Boxing** (`c-nan-boxing/fib-recursive/`)
  - Uses article-accurate NaN boxing with NANISH_MASK, INTEGER_MASK
  - 32-bit integers, ~0.006 seconds for n=30
  - Files: `fib.c`, `nanbox.h`, `Makefile`

- ✅ **C NaN Boxing + GC + Tiny Strings** (`c-nan-boxing-2/fib-recursive/`)
  - Same performance as c-nan-boxing for integer-only workload
  - Shadow stack garbage collection with zero memory leaks
  - Files: `fib.c`, `nanbox.h`, `nanbox_gc.h`, `gc.c`, `Makefile`
  
- ✅ **C++ Classic** (`cpp-classic/fib-recursive/`)
  - Uses authentic MiniScript::Value type system
  - ~0.01 seconds for n=30 (2x slower than NaN boxing, but still very fast)
  - Files: `fib.cpp`, `Makefile`

### 2. Levenshtein Distance (`levenshtein`)
**Purpose**: Tests string handling, list manipulation, and complex algorithms  
**Algorithm**: Edit distance with space-optimized DP (1D array)  
**Test cases**: "kitten"→"sitting" (3), Gettysburg Address variants (186)  

**Implementations completed:**
- ✅ **C NaN Boxing** (`c-nan-boxing/levenshtein/`)
  - Uses NaN boxing with string/list support via shared `nanbox.h`
  - ~0.013 seconds for full test suite (1.85x faster than C++)
  - Perfect accuracy on all test cases
  - Files: `levenshtein.c`, `Makefile`

- ✅ **C NaN Boxing + GC + Tiny Strings** (`c-nan-boxing-2/levenshtein/`)
  - **72% reduction in memory allocations** vs c-nan-boxing (52,952 vs 189,898 bytes)
  - ~0.016 seconds for full test suite with zero memory leaks
  - Perfect accuracy on all test cases
  - Files: `levenshtein.c`, `nanbox.h`, `nanbox_gc.h`, `gc.c`, `Makefile`

- ✅ **C++ Classic** (`cpp-classic/levenshtein/`)
  - Uses MiniScript String, ValueList types
  - ~0.024 seconds for full test suite
  - Perfect accuracy on all test cases
  - Files: `levenshtein.cpp`, `Makefile`

### 3. NumberWords (`numberWords`)
**Purpose**: Tests complex string processing, parsing, and bidirectional conversion  
**Algorithm**: Number ↔ English text conversion with full scale support  
**Test cases**: Round-trip conversion of 0-9999, plus edge cases (-1234, millions, etc.)  

**Implementations completed:**
- ✅ **C NaN Boxing** (`c-nan-boxing/numberWords/`)
  - Uses advanced string processing with `string_replace()` supporting multiple replacements
  - ~0.026 seconds for 10,000 round-trip conversions (1.77x faster than C++)
  - Perfect accuracy including hyphenated compound numbers
  - Files: `numberWords.c`, `Makefile`

- ✅ **C NaN Boxing + GC + Tiny Strings** (`c-nan-boxing-2/numberWords/`)
  - **17% reduction in memory allocations** vs c-nan-boxing (16,897,199 vs 20,431,272 bytes)
  - ~0.022 seconds for 10,000 round-trip conversions with zero memory leaks
  - Perfect accuracy including hyphenated compound numbers
  - Files: `numberWords.c`, `nanbox.h`, `nanbox_gc.h`, `gc.c`, `Makefile`

- ✅ **C++ Classic** (`cpp-classic/numberWords/`)
  - Uses MiniScript String, StringList with Split/Replace functions
  - ~0.046 seconds for 10,000 round-trip conversions
  - Perfect accuracy on all test cases including negatives and large numbers
  - Files: `numberWords.cpp`, `Makefile`

## Technical Implementation Details

### NaN Boxing Approach (`c-nan-boxing`)
- **Technique**: Based on [Piotr Duperas article](https://piotrduperas.com/posts/nan-boxing)
- **Key masks**: `NANISH_MASK`, `INTEGER_MASK`, `STRING_MASK`, `LIST_MASK`
- **Types supported**: 32-bit integers, doubles, strings, lists, nil
- **String features**: No UTF-8 support yet; split/replace operations correctly preserve empty tokens
- **List features**: Dynamic arrays with capacity management, mixed-type support
- **Performance**: Excellent across all workloads due to unified Value representation

### NaN Boxing + GC + Tiny Strings (`c-nan-boxing-2`)
- **Enhanced NaN Boxing**: All features of c-nan-boxing plus garbage collection and tiny string optimization
- **Tiny String Optimization**: Strings ≤5 characters stored directly in NaN-boxed values (zero heap allocation)
- **Shadow Stack GC**: Automatic memory management with scope-based root protection
- **Zero Memory Leaks**: Complete garbage collection eliminates all memory leaks
- **Performance Impact**: 17-72% reduction in memory allocations depending on workload
- **Backward Compatibility**: Existing code works unchanged with new optimizations

### MiniScript Classic Approach (`cpp-classic`)
- **Source**: Authentic MiniScript 1.x C++ codebase (modified for standalone compilation)
- **Key types**: `Value`, `String`, `ValueList`, `StringList` 
- **Features**: Full dynamic typing with reference counting, robust string handling, Unicode support
- **Performance**: Very good, about 2x slower than NaN boxing for pure numeric work
- **Strength**: Excellent string/list manipulation capabilities

### Build System Notes
- All implementations use `-O3` optimization
- **gitignore patterns**: `*_nanbox`, `*_classic` to exclude executables
- **Build products excluded**: `.o`, `.so`, `.dylib`, `.dll`, `.exe` files
- **Settings excluded**: `.claude/` directories

## Modified MiniScript Files
To make the MiniScript C++ code compile standalone, these modifications were made:

### Added Stub Headers:
- `UnitTest.h` - Mock unit test framework
- `MiniscriptErrors.h` - Exception classes (TypeException, IndexException, etc.)
- `MiniscriptIntrinsics.h` - Stub intrinsic functions  
- `MiniscriptTAC.h` - Context, Machine, Intrinsics stubs

### Commented Out:
- Unit test classes in `SimpleString.cpp`, `List.cpp`, `Dictionary.cpp`, `SplitJoin.cpp`
- Test registration calls (`RegisterUnitTest`)

### Key Fixes:
- Added missing exception types with `.raise()` methods
- Fixed `Context::GetVar()` signature for `LocalOnlyMode` enum
- Added stub `Machine` members (`numberType`, `stringType`, etc.)
- Added stub `Intrinsics` namespace functions

## Performance Summary

### Complete Benchmark Results
| Benchmark | MiniScript | C++ Classic | C NaN Boxing | NaN Boxing Speedup |
|-----------|------------|-------------|--------------|-------------------|
| **fib(30)** | N/A | 0.010s | **0.006s** | **1.67x faster** |
| **levenshtein** | ~31.1s | 0.024s | **0.013s** | **1.85x faster** |
| **numberWords(10k)** | 2.585s | 0.046s | **0.026s** | **1.77x faster** |

### Speedup vs MiniScript Reference
| Benchmark | C++ Classic vs MiniScript | C NaN Boxing vs MiniScript |
|-----------|---------------------------|----------------------------|
| **levenshtein** | **1,296x faster** | **2,393x faster** |
| **numberWords(10k)** | **56x faster** | **99x faster** |

Of course these results are preliminary, on limited benchmarks, and not really fair since the C NaN Boxing implementation leaks all allocated memory, and doesn't have any Unicode support.  But it does provide a sort of upper bound on what performance could possibly be.

## Next Steps / Future Work

### Performance enhancements

✅ **COMPLETED**: Tiny string optimization implemented in `c-nan-boxing-2`
- Strings ≤5 characters stored directly in NaN-boxed values
- 72% memory allocation reduction in string-heavy workloads
- Transparent to user code - automatic selection via `make_string()`

**Future enhancements** for c-nan-boxing-v3:
1. Add interning for (non-tiny) strings to reduce memory usage
2. Optimize string concatenation with rope data structures

### Extending functionality

- **Add Unicode support** to our string code (based on UnicodeUtil in the C++ code)
- **Add more benchmarks** from the MiniScript benchmark suite
- **Create additional type system implementations** (e.g. maps.)

### Consider other GC strategies

The world of GC is complex, and we've barely scratched the surface.  At some point we'll want to consider more prototype versions to explore:

- **Handles** to enable a "moving" GC (moves allocations around to stay compact)
- **Generations** to distinguish short-lived temps from longer-lived allocations
- **Incremental GC** rather than STW (Stop The World) collection


## Usage Notes
- Run `make` in any implementation directory to build
- Executables are auto-excluded by gitignore
- All implementations include correctness verification before timing
- Use `git add DynamicTypes/implementations/[path]/` to stage new work

## Key Lessons Learned
1. **NaN boxing is very fast** for numeric work but limited in type support
2. **MiniScript's type system** is surprisingly performant for a full-featured dynamic system
3. **String processing overhead** is significant but manageable with good C++ implementations
4. **Reference counting** in MiniScript adds minimal overhead for most use cases
5. **Build complexity** can be managed with careful dependency stubbing

---
*Last updated*: After completing tiny string optimization in c-nan-boxing-2
*Status*: 3/3 benchmarks implemented across 3 type systems (c-nan-boxing, c-nan-boxing-2, cpp-classic)
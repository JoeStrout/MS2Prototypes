# MS2Proto2 - Modular NaN-Boxing Dynamic Type System

This is a clean, modular reorganization of the c-nan-boxing-3 implementation, structured as a proper C library with clear API separation.

## Project Structure

```
MS2Proto2/
├── include/           # Public API headers
│   ├── nanbox.h      # Core NaN-boxing Value type and operations
│   ├── gc.h          # Garbage collector API  
│   ├── strings.h     # String implementation with interning
│   ├── lists.h       # List/array implementation
│   └── unicode.h     # Unicode/UTF-8 utilities
├── src/              # Implementation files
│   ├── nanbox.c      # Core NaN-boxing utilities
│   ├── gc.c          # Garbage collector implementation
│   ├── strings.c     # String operations and interning
│   ├── lists.c       # List operations
│   └── unicode.c     # Unicode/UTF-8 functions
├── tests/            # Unit tests
├── benchmarks/       # Performance benchmarks  
├── examples/         # Example programs
├── Makefile          # Build system
└── README.md         # This file
```

## Module Organization

The codebase is organized into clean, layered modules:

### Core Layer (`nanbox.h` + `nanbox.c`)
- **Value type**: `typedef uint64_t Value`
- **NaN-boxing masks**: Type tagging constants
- **Type checking**: `is_int()`, `is_string()`, `is_list()`, etc.
- **Value creation**: `make_int()`, `make_double()`, `make_nil()`
- **Value extraction**: `as_int()`, `as_double()`, `as_pointer()`

### Unicode Layer (`unicode.h` + `unicode.c`) 
- **UTF-8 utilities**: Character counting, encoding, decoding
- **No dependencies**: Uses only standard C libraries

### GC Layer (`gc.h` + `gc.c`)
- **Shadow stack GC**: Precise garbage collection with explicit root tracking
- **API**: `gc_init()`, `gc_allocate()`, `gc_collect()`, `GC_PUSH_SCOPE()`, etc.
- **Dependencies**: Core nanbox types

### String Layer (`strings.h` + `strings.c`)
- **Tiny string optimization**: ≤5 characters stored in NaN value
- **String interning**: Automatic interning for strings <128 bytes (immortal)
- **Operations**: Creation, equality, concatenation, splitting, etc.
- **Dependencies**: Core, GC, Unicode

### List Layer (`lists.h` + `lists.c`)
- **Dynamic arrays**: Automatic capacity management
- **Operations**: Creation, access, modification, searching
- **Dependencies**: Core, GC, Strings (for equality)

## Key Features

✅ **Modular Design**: Clear separation of concerns with minimal dependencies  
✅ **NaN Boxing**: 64-bit Values encoding multiple types efficiently  
✅ **Garbage Collection**: Mark-and-sweep with shadow stack root tracking  
✅ **Tiny String Optimization**: Short strings stored directly in Values  
✅ **String Interning**: Automatic interning with O(1) equality via `malloc()` (immortal)  
✅ **Unicode Support**: UTF-8 aware string operations  
✅ **Clean API**: Public headers define clear interfaces  

## Build System

The project uses a simple Makefile:

```bash
make              # Build core library (libms2proto2.a)
make tests        # Build test programs
make benchmarks   # Build benchmark programs  
make clean        # Clean all build artifacts
```

## Usage Example

```c
#include "nanbox.h"
#include "gc.h"
#include "strings.h"

int main() {
    gc_init();
    
    GC_PUSH_SCOPE();
    GC_LOCALS(str1, str2, result);
    
    str1 = make_string("Hello");
    str2 = make_string("World");
    result = string_concat(str1, str2);
    
    printf("Result: %s\n", as_cstring(result));
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 0;
}
```

## Development Status

🔄 **IN PROGRESS**: Core structure complete, working on build system integration

### Completed
- ✅ Folder structure and header design
- ✅ Core NaN-boxing module  
- ✅ Unicode utilities module
- ✅ Garbage collector module
- ✅ Basic strings module structure
- ✅ Basic lists module structure
- ✅ Build system framework

### TODO
- 🔲 Complete strings module implementation (fix missing functions)
- 🔲 Port and test benchmarks (fibonacci, levenshtein, numberWords) 
- 🔲 Create comprehensive test suite
- 🔲 Performance validation vs c-nan-boxing-3
- 🔲 Documentation and examples

## Design Philosophy

This reorganization prioritizes:

1. **Clear Separation**: Each module has a single responsibility
2. **Minimal Dependencies**: Modules depend only on what they need  
3. **Public APIs**: Clean header files define the interface contracts
4. **Testability**: Modular structure enables focused unit testing
5. **Maintainability**: Well-organized code for long-term development

The goal is to create a clean foundation for the MiniScript 2.0 dynamic type system that can be easily understood, tested, and extended.
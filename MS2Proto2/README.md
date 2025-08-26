# MS2Proto2 - Modular NaN-Boxing Dynamic Type System

This is a clean, modular reorganization of the c-nan-boxing-3 implementation, structured as a proper C library with clear API separation.

## Project Structure

```
MS2Proto2/
├── include/            # Public API headers
│   ├── types/         # Core Value type, including GC system
│   │    ├── nanbox.h  # Core NaN-boxing Value type and operations
│   │    ├── gc.h      # Garbage collector API  
│   │    ├── strings.h # String implementation with interning
│   │    ├── lists.h   # List/array implementation
│   │    └── unicode.h # Unicode/UTF-8 utilities
│   └── vm/            # Virtual machine
|         └── vm.h     # VM interfaces
├── src/               # Implementation files
│   │    ├── nanbox.c  # Core NaN-boxing utilities
│   │    ├── gc.c      # Garbage collector implementation
│   │    ├── strings.c # String operations and interning
│   │    ├── lists.c   # List operations
│   │    └── unicode.c # Unicode/UTF-8 functions
│   └── vm/            # Virtual machine
│         └── vm.c     # VM implementation
├── main.c             # main program
├── tests/             # Unit tests
├── benchmarks/        # Performance benchmarks  
├── examples/          # Example programs
├── Makefile           # Build system
└── README.md          # This file
```

## Module Organization

The codebase is organized into clean, layered modules:

### Core Layer (`nanbox.h` + `nanbox.c`)
- **Value type**: `typedef uint64_t Value`
- **NaN-boxing masks**: Type tagging constants
- **Type checking**: `is_int()`, `is_string()`, `is_list()`, etc.
- **Value creation**: `make_int()`, `make_double()`, `make_null()`
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

### VM (`vm.h` + `vm.c`)
- **Register VM**: Each function uses a contiguous slice of a single `Value*` stack; arguments/results live in the stack too
- **Fixed 4-byte Opcodes**: generally following an INSTRUCTION, A, B, C format (with A specifying results an B, C specifying operands, all as register numbers)
- **Fast Dispatch**: where compiler support is available, instructions are dispatched using computed `goto`, falling back to standard `switch` otherwise
- **Efficient Calls**: VM structure is designed to make pushing/popping calls very efficient

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

## Development Status

🔄 **IN PROGRESS**: Core structure complete, including a minimal but functional VM.  Program code must be loaded directly, opcode by opcode.  Only a handful of value operations (e.g. addition/subtraction) are implemented, and even those are not yet implemented for all types.

Currently the interval VM supports 32-bit integers as a type, even though this does not correspond directly to any MiniScript type.  We might keep this as an optimization, or throw it out for simplicity.  Time will tell.

### TODO
- 🔲 Implement missing Value functionality (adding strings, etc.)
- 🔲 Implement a simple assembly language (serialization of VM instructions)
- 🔲 Implement current 3 benchmarks as assembly scripts


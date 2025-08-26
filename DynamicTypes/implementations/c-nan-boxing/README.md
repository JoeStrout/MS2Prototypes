# C NaN-Boxing Implementation with Shadow Stack GC

This folder implements a NaN-boxing scheme in C with a complete garbage collection system. Reference:
	https://piotrduperas.com/posts/nan-boxing

## Current Implementation Status

**✅ COMPLETE**: Full NaN-boxing dynamic type system with shadow stack garbage collection

### Core Features

- **NaN-Boxing**: 64-bit values encoding integers, doubles, strings, lists, and special values
- **Shadow Stack GC**: Automatic garbage collection using Value* root tracking
- **Type System**: Complete support for nil, booleans, integers, doubles, strings, and lists
- **Memory Management**: Mark-and-sweep garbage collector with scope-based protection

### Supported Types

- **Integers**: 32-bit signed integers stored directly in NaN-boxed values
- **Doubles**: Full 64-bit floating point numbers
- **Strings**: Heap-allocated, garbage-collected C strings
- **Lists**: Dynamic arrays of Values, garbage-collected
- **Special Values**: nil, true, false

### Shadow Stack GC System

The garbage collector uses a "shadow stack" approach where local variables are automatically tracked:

```c
void my_function() {
    GC_PUSH_SCOPE();           // Start scope
    
    Value str = make_null();    // Declare local variables
    Value list = make_null();
    
    GC_PROTECT(&str);          // Protect from GC (pass pointer!)
    GC_PROTECT(&list);
    
    str = make_string("hello"); // Safe to reassign
    list = make_list(10);
    
    GC_POP_SCOPE();            // End scope, auto-unprotect
}
```

### Working Benchmarks

All three benchmarks are implemented and tested:

1. **fib-recursive**: Recursive Fibonacci using NaN-boxed integers
2. **levenshtein**: String edit distance with dynamic programming  
3. **numberWords**: Integer-to-English conversion with complex string operations

### Development/Debug Features

The following compile-time flags are available for development (currently disabled):

- `GC_AGGRESSIVE`: Triggers collection on every allocation (stress testing)
- `GC_DEBUG`: Overwrites freed memory with 0xDEADBEEF pattern to catch stale pointers

### Architecture Notes

- Uses mark-and-sweep garbage collection with scope-based root management
- Memory allocation happens before collection (fixed critical timing bug)
- All local Value variables must be protected via GC_PROTECT(&var)
- Collection only occurs at safe points (allocation, explicit gc_collect())
- Zero memory leaks achieved in all benchmarks

### Files

- `nanbox.h`: Core NaN-boxing type definitions and operations
- `nanbox_gc.h`: Shadow stack GC system and macros  
- `gc.c`: Garbage collector implementation
- `fib-recursive/`, `levenshtein/`, `numberWords/`: Complete working benchmarks
- `test_shadow_stack.c`: GC system test suite

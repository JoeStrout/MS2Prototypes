# Garbage Collection Implementation Plan

## Overview
Add memory management and garbage collection to the C NaN boxing implementation to eliminate memory leaks while maintaining performance advantages over the C++ classic approach.

## Phased Implementation Strategy

### ✅ Phase 1: Unified Tracing GC with Precise Root Tracking
**Goal**: Implement mark-and-sweep garbage collection for both strings and lists

**Approach**:
- Single GC strategy for all managed objects (strings and lists)
- Precise root tracking using explicit root set management
- Mark phase: traverse from known roots, mark all reachable objects
- Sweep phase: free all unmarked objects

**Rationale**:
- Avoids reference counting overhead on every Value operation
- Simpler than hybrid approaches - one GC algorithm to implement and tune
- Precise tracking gives predictable behavior and easier debugging
- Clean performance baseline without conservative scanning overhead

### Phase 2: Add TinyString
**Goal**: Optimize operations on strings small enough to fit in a NaN box, avoiding allocations on these entirely

**Approach**:
- Define another type and bitmask for "tiny strings"
- `make_string` returns this type or the original string type as needed
- `is_anystring` returns true for any string; `is_tinystring` returns true for tiny ones; `is_string` returns true for big strings.  Most user code only checks `is_anystring`, and string-handling APIs work for either string type, so that user code doesn't need to know or care exactly what string type it's working with.

### Phase 3: Add String Interning
**Goal**: Optimize string operations through interning with GC integration

**Approach**:
- Maintain weak hash table of interned strings
- String equality becomes pointer comparison
- During GC: mark reachable interned strings, remove unreachable ones from table
- Memory still gets freed for unreachable strings

**Benefits**:
- Faster string equality comparisons
- Reduced memory usage for duplicate strings
- Common optimization in dynamic language implementations

### Phase 4: Conservative Stack Scanning (If Needed)
**Goal**: Simplify programming model if precise tracking proves cumbersome

**Approach**:
- Scan stack/registers for values matching STRING_MASK or LIST_MASK bit patterns
- Extract pointers and verify they point to valid heap objects
- NaN boxing bit patterns make this much safer than typical conservative GC

**Advantages of NaN boxing for conservative scanning**:
- STRING_MASK and LIST_MASK provide specific bit patterns
- Regular doubles and untagged pointers won't match these patterns
- Significantly fewer false positives than traditional conservative GC

## Performance Measurement Strategy
- Benchmark after each phase using existing test suite:
  - fib-recursive (minimal GC impact)
  - levenshtein (moderate string/list allocation)  
  - numberWords (heavy string processing)
- Compare against current leaky implementation and C++ classic
- Measure both execution time and memory usage

## Technical Considerations
- **Root management**: Functions push/pop local Values to root set
- **Collection triggers**: Allocation-based thresholds, with manual trigger for benchmarking
- **Object headers**: Minimal mark bits, size information for sweep phase
- **Heap organization**: Simple free-list or bump-pointer allocation

## Success Criteria
- Zero memory leaks on all benchmarks
- Performance within 10-20% of current leaky implementation
- Maintain advantage over C++ classic approach
- Clean, maintainable codebase ready for future extensions
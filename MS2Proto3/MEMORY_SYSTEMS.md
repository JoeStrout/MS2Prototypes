# Memory Systems in MS2Proto3

This document describes the multiple memory management systems used in MS2Proto3 and how they interact.

## Overview

MS2Proto3 uses **four distinct memory systems** for different purposes:

1. **GC (Garbage Collector)** - For runtime MiniScript values
2. **Intern Table** - For frequently-used small runtime strings
3. **String Pool** - For host/compiler strings (C# `String` class)
4. **MemPool** - General purpose pooled allocation for host code

## 1. GC System (Garbage Collector)

**Location:** `cpp/core/gc.c`, `cpp/core/gc.h`

**Purpose:** Manages memory for MiniScript runtime values (strings, lists, maps).

**Implementation:**
- Mark-and-sweep garbage collector
- Shadow stack for root set tracking
- Each allocation has a `GCObject` header (next pointer, marked flag, size)
- All objects linked in `gc.all_objects` list
- Allocation: `gc_allocate(size)` - adds GCObject header, returns pointer to data
- Collection triggered when `gc.bytes_allocated > gc.gc_threshold`

**Used for:**
- **Heap strings** (strings > 128 bytes)
- **ValueList** structures and their item arrays
- **ValueMap** structures and their entry arrays
- Any dynamically-sized runtime data structures

**Lifetime:** Objects live until unreachable from root set, then collected during GC sweep.

**Key insight:** The `marked` flag is only valid during a collection cycle - it's set during mark phase and cleared during sweep phase.

## 2. Intern Table System

**Location:** `cpp/core/value_string.c` (lines 16-193)

**Purpose:** Deduplicate small, frequently-used runtime strings to save memory.

**Implementation:**
- Static hash table: `InternEntry* intern_table[INTERN_TABLE_SIZE]` (1024 buckets)
- Each `InternEntry` contains:
  - `Value string_value` - the interned string (a heap string Value)
  - `struct InternEntry* next` - collision chain pointer
- `InternEntry` structs allocated with `malloc()` (NOT gc_allocate)
- Hash-based lookup with collision chaining

**Used for:**
- Strings < 128 bytes (INTERN_THRESHOLD)
- Automatically used by `make_string()` for small strings
- Examples: keywords, common identifiers, short literals

**Lifetime:** **Immortal** - never freed during program execution. These are deliberately leaked to avoid the cost of reference counting or managing lifetimes.

**Key characteristics:**
- NOT managed by GC (comment at line 188: "this is not GC'd - it's part of the intern table")
- The `StringStorage` structures themselves ARE still reachable and won't be collected
- Provides O(1) string deduplication for small strings

**Optimization note:** Line 218 comment suggests future possibility of using MemPool instead of malloc for eventual cleanup.

## 3. String Pool System

**Location:** `cpp/core/StringPool.h`, `cpp/core/StringPool.cpp`

**Purpose:** String interning for **host** code (the C# `String` class, assembler, compiler, etc.).

**Implementation:**
- 256 separate pools (indexed 0-255)
- Each pool has hash table with 256 buckets
- Entries stored as `MemRef` (pool number + index into MemPool)
- Uses `MemPool` for underlying allocation

**Used by:**
- C# `String` class (transpiled to C++)
- Assembler (function names, labels, etc.)
- Any host code using the `String` class
- VMVis display strings (uses temporary pool, cleared each frame)

**Lifetime:**
- Strings persist until their pool is explicitly cleared with `StringPool::clearPool(poolNum)`
- Pool 0 is the default and typically long-lived
- Temporary pools can be allocated/cleared for transient strings

**Separation from runtime:** This system is completely separate from the Value/GC system. Host strings are not MiniScript values.

## 4. MemPool System

**Location:** `cpp/core/MemPool.h`, `cpp/core/MemPool.cpp`

**Purpose:** Pooled memory allocation for host code data structures.

**Implementation:**
- 256 independent pools (indexed 0-255)
- Each pool maintains array of blocks
- Blocks never freed individually - entire pool cleared at once
- Reference via `MemRef` (pool number + block index)

**Used for:**
- StringPool's internal structures (hash entries, StringStorage)
- Any host code needing pooled allocation
- Temporary data structures that can be bulk-freed

**Lifetime:** Blocks persist until entire pool is cleared with `MemPoolManager::clearPool(poolNum)`

## String Types Summary

### Tiny Strings (≤ 5 bytes)
- **Storage:** Inline in NaN-boxed Value (no allocation)
- **Examples:** "a", "x", "true", "null"
- **Lifetime:** Lives as long as the Value exists
- **System:** None (embedded in Value itself)

### Interned Strings (< 128 bytes)
- **Storage:** `malloc()` for StringStorage, stored in `intern_table`
- **Examples:** "count", "name", "print", short literals
- **Lifetime:** Immortal (never freed)
- **System:** Intern Table

### GC Heap Strings (≥ 128 bytes)
- **Storage:** `gc_allocate()` for StringStorage
- **Examples:** Long string literals, concatenation results
- **Lifetime:** GC-managed (collected when unreachable)
- **System:** GC

### Host Strings (C# `String` class)
- **Storage:** MemPool allocation, interned in StringPool
- **Examples:** Function names, labels, compiler strings, debug output
- **Lifetime:** Pool lifetime (until explicitly cleared)
- **System:** StringPool + MemPool

## Memory System Interactions

### Clear Separation
- **Runtime (GC + Intern Table)** ↔ **Host (StringPool + MemPool)** systems are independent
- Converting between them requires explicit string copying
- Host strings are never seen by the GC
- Runtime Values don't use StringPool

### Temporary Pool Strategy
Several subsystems use temporary pools to reduce memory pressure:

1. **Assembler** (implemented):
   - Allocates temp pool at start of `Assemble()`
   - Function names re-interned into pool 0 before clearing
   - Temp pool cleared after assembly complete

2. **VMVis** (implemented):
   - Allocates temp pool in constructor
   - Switches to it during `UpdateDisplay()`
   - Clears after each frame rendered

3. **App Main** (implemented):
   - Uses temp pool for file reading and command-line processing
   - Switches back to pool 0 before running VM

## Debugging Memory

### For GC Objects
Use `gc_dump_objects()` (to be implemented):
- Shows all objects in `gc.all_objects` list
- Hex/ASCII dump of object data
- Mark status (if mark phase run first)

### For Interned Strings
Use `dump_intern_table()` (to be implemented):
- Walk `intern_table[1024]` array
- Follow collision chains
- Show each interned string value

### For StringPool
Use `StringPool::dumpAllPoolState()` (implemented):
- Shows all strings in each initialized pool
- Pool statistics (entries, bins, chain lengths)
- Already working with pagination and escape sequences

### For MemPool
Use `MemPoolManager` statistics:
- Per-pool block counts and sizes
- Total memory allocated

## Design Rationale

### Why Multiple Systems?

1. **Performance:** Different allocation patterns optimized differently
   - GC for complex object graphs
   - Interning for deduplication
   - Pools for bulk allocation/deallocation

2. **Lifetime Management:** Different data has different lifetimes
   - Runtime values: GC-managed
   - Small strings: Immortal (intern table)
   - Compiler data: Scoped (pools)

3. **Language Integration:** Clean separation between host and runtime
   - C# String class uses StringPool
   - MiniScript strings use GC/intern table
   - No mixing or confusion

### Historical Note
The dual-language (C#/C++) architecture necessitates host-level string management (StringPool) that is separate from the runtime's Value-based string system.

---

*Last updated: [Date] - Added comprehensive documentation of all memory systems*

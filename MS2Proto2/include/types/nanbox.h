// nanbox.h
//
// Purpose: this module defines the Value type, an 8-byte NaN-box representation
// of our dynamic type.  This Value will always be either a valid Double, or
// an actual numeric NaN, or a representation of some other type (possibly pointing
// to additional data in the heap, in the case of strings, lists, and maps).
//
// Because a Value may point to garbage-collected memory allocations, care must be
// taken to protect local Value variables (see GC_USAGE.md).

#ifndef NANBOX_H
#define NANBOX_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// Core NaN-boxing dynamic type system
// Based on: https://piotrduperas.com/posts/nan-boxing

typedef uint64_t Value;

// NaN-boxing masks and constants
#define NANISH_MASK        0xffff000000000000ULL
#define NANISH             0x7ffc000000000000ULL
#define INTEGER_MASK       0x7ffc000000000000ULL
#define MAP_MASK           0xfffb000000000000ULL
#define STRING_MASK        0xfffe000000000000ULL
#define LIST_MASK          0xfffd000000000000ULL

// Tiny string support - uses STRING_MASK with special encoding
#define TINY_STRING_MASK   0xffff000000000000ULL  // Top bit set for tiny strings
#define TINY_STRING_MAX_LEN 5                     // Max 5 chars in 40 bits (bits 8-47)

#define NULL_VALUE         0x7ffe000000000000ULL

// Core type checking functions
static inline bool is_nil(Value v) {
    return v == NULL_VALUE;
}

static inline bool is_int(Value v) {
    return (v & NANISH_MASK) == INTEGER_MASK;
}

static inline bool is_tiny_string(Value v) {
    return (v & TINY_STRING_MASK) == TINY_STRING_MASK;
}

static inline bool is_heap_string(Value v) {
    return (v & STRING_MASK) == STRING_MASK && !is_tiny_string(v);
}

static inline bool is_string(Value v) {
    return is_tiny_string(v) || is_heap_string(v);
}

static inline bool is_list(Value v) {
    return (v & NANISH_MASK) == LIST_MASK;
}

static inline bool is_map(Value v) {
    return (v & NANISH_MASK) == MAP_MASK;
}

static inline bool is_double(Value v) {
    // A value is a double if it doesn't have any of our NaN-boxing type masks
    // This means it's a normal IEEE 754 double value
    return !is_nil(v) && !is_int(v) && !is_tiny_string(v) && !is_heap_string(v) && !is_list(v) && !is_map(v);
}

static inline bool is_number(Value v) {
    return is_int(v) || is_double(v);
}

// Core value creation functions
static inline Value make_nil(void) {
    return NULL_VALUE;
}

static inline Value make_int(int32_t i) {
    return INTEGER_MASK | (uint64_t)(uint32_t)i;
}

static inline Value make_double(double d) {
	// This looks expensive, but it's the only 100% portable way, and modern
	// compilers will recognize it as a bit-cast and emit a single move instruction.
    Value v;
    memcpy(&v, &d, sizeof v);   // bitwise copy; aliasing-safe
    return v;
}

// Note: make_map() will be implemented in the maps module

// Core value extraction functions
static inline int32_t as_int(Value v) {
    return (int32_t)v;
}

static inline double as_double(Value v) {
	// See comments in make_double.
    double d;
    memcpy(&d, &v, sizeof d);   // aliasing-safe bit copy
    return d;
}

// Note: as_map() will be implemented in the maps module

// Utility functions for accessing tiny string data within Value
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define GET_VALUE_DATA_PTR(v_ptr) ((char*)(v_ptr))
    #define GET_VALUE_DATA_PTR_CONST(v_ptr) ((const char*)(v_ptr))
#else
    #define GET_VALUE_DATA_PTR(v_ptr) (((char*)(v_ptr)) + 2)
    #define GET_VALUE_DATA_PTR_CONST(v_ptr) (((const char*)(v_ptr)) + 2)
#endif

// Arithmetic operations (implemented in nanbox.c)
Value value_add(Value a, Value b);
Value value_sub(Value a, Value b);
bool value_lt(Value a, Value b);

// Debug utility functions (implemented in nanbox.c)
void debug_print_value(Value v);
const char* value_type_name(Value v);

#endif // NANBOX_H
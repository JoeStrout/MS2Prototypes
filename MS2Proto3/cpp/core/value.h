// value.h
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
#include "hashing.h"

#ifdef __cplusplus
extern "C" {
#endif

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

static inline bool value_identical(Value a, Value b) {
	return a == b;
}

// Core type checking functions
static inline bool is_null(Value v) {
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
    return !is_null(v) && !is_int(v) && !is_tiny_string(v) && !is_heap_string(v) && !is_list(v) && !is_map(v);
}

static inline bool is_number(Value v) {
    return is_int(v) || is_double(v);
}

bool is_truthy(Value v);

// Core value creation functions
static inline Value make_null(void) {
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

// Forward declarations for string functions (implemented in value_string.h/c)
extern Value string_concat(Value a, Value b);
extern Value make_string(const char* str);
extern const char* get_string_data_zerocopy(const Value* v_ptr, int* out_len);
extern int string_compare(Value a, Value b);

// Conversion functions

Value to_string(Value v);
Value to_number(Value v);

// Arithmetic operations (inlined for performance)
static inline Value value_add(Value a, Value b) {
    // Handle integer + integer case
    if (is_int(a) && is_int(b)) {
        // Use int64_t to detect overflow
        int64_t result = (int64_t)as_int(a) + (int64_t)as_int(b);
        if (result >= INT32_MIN && result <= INT32_MAX) {
            return make_int((int32_t)result);
        } else {
            // Overflow to double
            return make_double((double)result);
        }
    }
    
    // Handle mixed integer/double or double/double cases
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return make_double(da + db);
    }
    
    // Handle string concatenation

    if (is_string(a)) {
        if (is_string(b)) return string_concat(a, b);
        if (is_int(b) || is_double(b)) return string_concat(a, to_string(b));
	} else if (is_string(b)) {
        if (is_int(a) || is_double(a)) return string_concat(to_string(a), b);
    }
    
    // For now, return nil for unsupported operations
    return make_null();
}

static inline Value value_sub(Value a, Value b) {
    // Handle integer - integer case
    if (is_int(a) && is_int(b)) {
        // Use int64_t to detect overflow/underflow
        int64_t result = (int64_t)as_int(a) - (int64_t)as_int(b);
        if (result >= INT32_MIN && result <= INT32_MAX) {
            return make_int((int32_t)result);
        } else {
            // Overflow/underflow to double
            return make_double((double)result);
        }
    }
    
    // Handle mixed integer/double or double/double cases
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return make_double(da - db);
    }
    
    // Return nil for unsupported operations
    return make_null();
}

// Forward declaration for string equality function
extern bool string_equals(Value a, Value b);

// Most critical comparison function (inlined for performance)
static inline bool value_lt(Value a, Value b) {
    // Handle numeric comparisons
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return da < db;
    }
    
    // Handle string comparisons (Unicode-aware)
    if (is_string(a) && is_string(b)) {
        return string_compare(a, b) < 0;
    }
    
    // For now, return false for unsupported comparisons
    return false;
}

extern Value value_mult_nonnumeric(Value a, Value b);
static inline Value value_mult(Value a, Value b) {
    // Handle integer * integer case
    if (is_int(a) && is_int(b)) {
        // Use int64_t to detect overflow
        int64_t result = (int64_t)as_int(a) * (int64_t)as_int(b);
        if (result >= INT32_MIN && result <= INT32_MAX) {
            return make_int((int32_t)result);
        } else {
            // Overflow to double
            return make_double((double)result);
        }
    }
    
    // Handle mixed integer/double or double/double cases
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return make_double(da * db);
    }
    
    // Everything else, go to the non-numeric handler
    return value_mult_nonnumeric(a, b);
}

static inline Value value_div(Value a, Value b) {
    // Handle integer / integer case
    if (is_int(a) && is_int(b)) {
        // Use int64_t to detect overflow
        int64_t result = (int64_t)as_int(a) / (int64_t)as_int(b);
        if (result >= INT32_MIN && result <= INT32_MAX) {
            return make_int((int32_t)result);
        } else {
            // Overflow to double
            return make_double((double)result);
        }
    }
    
    // Handle mixed integer/double or double/double cases
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return make_double(da / db);
    // Handle string / number
    } else if (is_string(a) && is_number(b)) {
    	// We'll just call through to value_mult for this, with a factor of 1/b.
    	return value_mult_nonnumeric(a, value_div(make_double(1), b));
    }
    return make_null();
}

static inline Value value_mod(Value a, Value b) {
    // Handle integer % integer case
    if (is_int(a) && is_int(b)) {
        // Use int64_t to detect overflow
        int64_t result = (int64_t)as_int(a) % (int64_t)as_int(b);
        if (result >= INT32_MIN && result <= INT32_MAX) {
            return make_int((int32_t)result);
        } else {
            // Overflow to double
            return make_double((double)result);
        }
    }
    
    // Handle mixed integer/double or double/double cases
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return make_double(fmod(da, db));
    }
    return make_null();
}

// Value comparison (most critical ones inlined above, others implemented in value.c)
bool value_equal(Value a, Value b);
bool value_le(Value a, Value b);
static inline bool value_gt(Value a, Value b) { return !value_le(a, b); }
static inline bool value_ge(Value a, Value b) { return !value_lt(a, b); }


// Bit-wise operations
Value value_and(Value a, Value b);
Value value_or(Value a, Value b);
Value value_xor(Value a, Value b);
Value value_unary(Value a);  // ~ (not)
Value value_shr(Value v, int shift);
Value value_shl(Value v, int shift);

// Hash function for Values
uint32_t value_hash(Value v);

// Debug utility functions (implemented in value.c)
void debug_print_value(Value v);
const char* value_type_name(Value v);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // NANBOX_H
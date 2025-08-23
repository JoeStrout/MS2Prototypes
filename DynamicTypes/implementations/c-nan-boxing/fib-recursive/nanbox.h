#ifndef NANBOX_H
#define NANBOX_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef uint64_t Value;

// Masks and constants from the article
#define NANISH_MASK        0xffff000000000000ULL
#define NANISH             0x7ffc000000000000ULL
#define BOOLEAN_MASK       0x7ffe000000000002ULL
#define INTEGER_MASK       0x7ffc000000000000ULL
#define OBJECT_MASK        0xfffc000000000000ULL
#define STRING_MASK        0xfffe000000000000ULL

#define TRUE_VALUE         (BOOLEAN_MASK | 3)
#define FALSE_VALUE        (BOOLEAN_MASK | 2)
#define NULL_VALUE         0x7ffe000000000000ULL

// Type checking macros
static inline bool is_number(Value v) {
    return (v & NANISH) != NANISH;
}

static inline bool is_nil(Value v) {
    return v == NULL_VALUE;
}

static inline bool is_bool(Value v) {
    return (v & BOOLEAN_MASK) == BOOLEAN_MASK;
}

static inline bool is_int(Value v) {
    return (v & NANISH_MASK) == INTEGER_MASK;
}

static inline bool is_pointer(Value v) {
    return (v & NANISH_MASK) == OBJECT_MASK;
}

// Value creation functions
static inline Value make_number(double d) {
    union { double d; uint64_t i; } u;
    u.d = d;
    return u.i;
}

static inline double as_number(Value v) {
    union { uint64_t i; double d; } u;
    u.i = v;
    return u.d;
}

static inline Value make_nil() {
    return NULL_VALUE;
}

static inline Value make_bool(bool b) {
    return b ? TRUE_VALUE : FALSE_VALUE;
}

static inline bool as_bool(Value v) {
    return (v & 0x1) != 0;
}

static inline Value make_int(int32_t i) {
    return INTEGER_MASK | (uint64_t)(uint32_t)i;
}

static inline int32_t as_int(Value v) {
    return (int32_t)v;
}

static inline Value make_pointer(void* ptr) {
    return OBJECT_MASK | ((uintptr_t)ptr & 0xFFFFFFFFFFFFULL);
}

static inline void* as_pointer(Value v) {
    return (void*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
}

static inline bool is_truthy(Value v) {
    if (is_nil(v)) return false;
    if (is_bool(v)) return as_bool(v);
    if (is_number(v)) return as_number(v) != 0.0;
    if (is_int(v)) return as_int(v) != 0;
    return true;
}

static inline bool values_equal(Value a, Value b) {
    if (is_number(a) && is_number(b)) {
        return as_number(a) == as_number(b);
    }
    return a == b;
}

#endif
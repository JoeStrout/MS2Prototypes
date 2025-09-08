// value.c
//
// Core NaN-boxing implementation utilities
// Most functionality is in value.h as inline functions

#include "value.h"
#include "value_string.h"
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

// Debug utilities for Value inspection
void debug_print_value(Value v) {
    if (is_null(v)) {
        printf("null");
    } else if (is_int(v)) {
        printf("int(%d)", as_int(v));
    } else if (is_double(v)) {
        printf("double(%g)", as_double(v));
    } else if (is_tiny_string(v)) {
        const char* data = GET_VALUE_DATA_PTR_CONST(&v);
        int len = (int)(unsigned char)data[0];
        printf("tiny_string(len=%d,\"", len);
        for (int i = 0; i < len && i < TINY_STRING_MAX_LEN; i++) {
            char c = data[1 + i];
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf("\\x%02x", (unsigned char)c);
            }
        }
        printf("\")");
    } else if (is_heap_string(v)) {
        uintptr_t ptr = (uintptr_t)(v & 0xFFFFFFFFFFFFULL);
        printf("heap_string(ptr=0x%llx)", (unsigned long long)ptr);
    } else if (is_list(v)) {
        uintptr_t ptr = (uintptr_t)(v & 0xFFFFFFFFFFFFULL);
        printf("list(ptr=0x%llx)", (unsigned long long)ptr);
    } else if (is_map(v)) {
        uintptr_t ptr = (uintptr_t)(v & 0xFFFFFFFFFFFFULL);
        printf("map(ptr=0x%llx)", (unsigned long long)ptr);
    } else {
        printf("unknown(0x%016llx)", v);
    }
}

// Value type name for debugging
const char* value_type_name(Value v) {
    if (is_null(v)) return "nil";
    if (is_int(v)) return "int";
    if (is_double(v)) return "double";
    if (is_tiny_string(v)) return "tiny_string";
    if (is_heap_string(v)) return "heap_string";
    if (is_list(v)) return "list";
    if (is_map(v)) return "map";
    return "unknown";
}

// Arithmetic operations for VM support
// Note: value_add() and value_sub() are now inlined in value.h

Value value_mult(Value a, Value b) {
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
    
    // Handle string repetition: string * int or int * string
    if (is_string(a) && is_int(b)) {
        int count = as_int(b);
        if (count <= 0) return make_string("");
        if (count == 1) return a;
        
        // Build repeated string
        Value result = a;
        for (int i = 1; i < count; i++) {
            result = string_concat(result, a);
        }
        return result;
        // ToDo: handle fractional b.
    }
    
    // For now, return nil for unsupported operations
    return make_null();
}

Value value_div(Value a, Value b) {
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
    }
    
    // TODO: Handle string concatenation, etc.
    // For now, return nil for unsupported operations
    return make_null();
}

Value value_mod(Value a, Value b) {
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
    
    // TODO: Handle string concatenation, etc.
    // For now, return nil for unsupported operations
    return make_null();
}

// Note: value_lt() is now inlined in value.h

bool value_le(Value a, Value b) {
    // Handle numeric comparisons
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return da <= db;
    }
    
    // Handle string comparisons (Unicode-aware)
    if (is_string(a) && is_string(b)) {
        return string_compare(a, b) <= 0;
    }
    
    // For now, return false for unsupported comparisons
    return false;
}

bool value_equal(Value a, Value b) {
	bool sameType = ((a & NANISH_MASK) == (b & NANISH_MASK));
	
    if (is_int(a) && sameType) {
        return as_int(a) == as_int(b);
    }
    if (is_double(a) && sameType) {
        return as_double(a) == as_double(b);
    }
    if (is_string(a) && sameType) {
        return string_equals(a, b);
    }
    // Mixed int/double comparison
    if (is_number(a) && is_number(b)) {
        double da = is_int(a) ? (double)as_int(a) : as_double(a);
        double db = is_int(b) ? (double)as_int(b) : as_double(b);
        return da == db;
    }
    // Nulls
    if (is_null(a) && sameType) {
    	return true;
    }
    // Different types or unsupported types
    return false;
}


// Bitwise AND
Value value_and(Value a, Value b) {
    if (is_int(a) && is_int(b)) {
        return make_int(as_int(a) & as_int(b));
    }
    return make_null();
}

// Bitwise OR
Value value_or(Value a, Value b) {
    if (is_int(a) && is_int(b)) {
        return make_int(as_int(a) | as_int(b));
    }
    return make_null();
}

// Bitwise XOR
Value value_xor(Value a, Value b) {
    if (is_int(a) && is_int(b)) {
        return make_int(as_int(a) ^ as_int(b));
    }
    return make_null();
}

// Bitwise NOT (unary)
Value value_unary(Value a) {
    if (is_int(a)) {
        return make_int(~as_int(a));
    }
    return make_null();
}

// Shift Right
Value value_shr(Value v, int shift) {
    if (!is_int(v)) {
        return make_null();
    }

    // Logical shift for unsigned behavior
    return make_int((uint32_t)as_int(v) >> shift);
}

Value value_shl(Value v, int shift) {
    if (!is_int(v)) {
        // Unsupported type for shift-left
        return make_null();
    }

    int64_t result = (int64_t)as_int(v) << shift;

    // Check for overflow beyond 32-bit signed integer range
    if (result >= INT32_MIN && result <= INT32_MAX) {
        return make_int((int32_t)result);
    } else {
        // Overflow: represent as double
        return make_double((double)result);
    }
}
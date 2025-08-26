// nanbox.c
//
// Core NaN-boxing implementation utilities
// Most functionality is in nanbox.h as inline functions

#include "../include/nanbox.h"
#include <stdio.h>

// Debug utilities for Value inspection
void debug_print_value(Value v) {
    if (is_nil(v)) {
        printf("nil");
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
    if (is_nil(v)) return "nil";
    if (is_int(v)) return "int";
    if (is_double(v)) return "double";
    if (is_tiny_string(v)) return "tiny_string";
    if (is_heap_string(v)) return "heap_string";
    if (is_list(v)) return "list";
    if (is_map(v)) return "map";
    return "unknown";
}
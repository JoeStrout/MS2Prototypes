#ifndef NANBOX_H
#define NANBOX_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "unicodeUtil.h"

// Forward declaration for GC function
void* gc_allocate(size_t size);

typedef uint64_t Value;

// Masks and constants from the article
#define NANISH_MASK        0xffff000000000000ULL
#define NANISH             0x7ffc000000000000ULL
#define INTEGER_MASK       0x7ffc000000000000ULL
#define OBJECT_MASK        0xfffc000000000000ULL
#define STRING_MASK        0xfffe000000000000ULL
#define LIST_MASK          0xfffd000000000000ULL

// Tiny string support - uses STRING_MASK with special encoding
#define TINY_STRING_MASK   0xffff000000000000ULL  // Top bit set for tiny strings
#define TINY_STRING_MAX_LEN 5                     // Max 5 chars in 40 bits (bits 8-47)

#define NULL_VALUE         0x7ffe000000000000ULL

// Type checking macros
static inline bool is_number(Value v) {
    return (v & NANISH) != NANISH;
}

static inline bool is_nil(Value v) {
    return v == NULL_VALUE;
}

static inline bool is_int(Value v) {
    return (v & NANISH_MASK) == INTEGER_MASK;
}

static inline bool is_pointer(Value v) {
    return (v & NANISH_MASK) == OBJECT_MASK;
}

static inline bool is_tiny_string(Value v) {
    return (v & TINY_STRING_MASK) == TINY_STRING_MASK;
}

static inline bool is_heap_string(Value v) {
    return (v & NANISH_MASK) == STRING_MASK && !is_tiny_string(v);
}

static inline bool is_string(Value v) {
    return is_tiny_string(v) || is_heap_string(v);
}

static inline bool is_list(Value v) {
    return (v & NANISH_MASK) == LIST_MASK;
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

// String structure and functions
// NOTE: len field represents BYTE length, not character length
typedef struct {
    int lenB;  // Length in bytes (was: len)
    char data[];
} String;

// Helper to get pointer to the 6-byte data area within a Value
// Handles endianness - returns pointer to the lower 6 bytes
static inline char* get_value_data_ptr(Value* v) {
    // On little-endian systems, the lower 6 bytes start at the beginning
    // On big-endian systems, they start 2 bytes in
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return (char*)v;
    #else
        return ((char*)v) + 2;  // Skip the high 2 bytes on big-endian
    #endif
}

// Helper to get const pointer to the 6-byte data area within a Value
static inline const char* get_value_data_ptr_const(const Value* v) {
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return (const char*)v;
    #else
        return ((const char*)v) + 2;
    #endif
}

static inline Value make_tiny_string(const char* str, int len) {
    // Create the value with proper type mask
    Value v = TINY_STRING_MASK;
    
    // Get pointer to the 6-byte data area and store length-prefixed string
    char* data = get_value_data_ptr(&v);
    data[0] = (char)len;  // Store length in first byte
    
    // Copy string data starting at byte 1
    for (int i = 0; i < len && i < TINY_STRING_MAX_LEN; i++) {
        data[1 + i] = str[i];
    }
    
    // Remaining bytes are already zero from TINY_STRING_MASK initialization
    // INVARIANT: All unused payload bytes in tiny strings are guaranteed to be zero
    
    return v;
}

static inline Value make_string(const char* str) {
    if (str == NULL) return make_nil();
    int lenB = strlen(str);  // Byte length
    
    // Use tiny string for short strings (in bytes)
    if (lenB <= TINY_STRING_MAX_LEN) {
        return make_tiny_string(str, lenB);
    }
    
    // Use heap string for longer strings
    String* s = (String*)gc_allocate(sizeof(String) + lenB + 1);
    s->lenB = lenB;  // Store byte length
    strcpy(s->data, str);
    return STRING_MASK | ((uintptr_t)s & 0xFFFFFFFFFFFFULL);
}

static inline String* as_string(Value v) {
    if (is_heap_string(v)) {
        return (String*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
    }
    return NULL; // Tiny strings don't have a String structure
}

// Buffer for tiny string conversion - thread-local would be better in real code
static char tiny_string_buffer[TINY_STRING_MAX_LEN + 1];

static inline const char* as_cstring(Value v) {
    if (!is_string(v)) return NULL;
    
    if (is_tiny_string(v)) {
        const char* data = get_value_data_ptr_const(&v);
        int len = (int)(unsigned char)data[0];
        
        // Copy to buffer and add null terminator
        for (int i = 0; i < len && i < TINY_STRING_MAX_LEN; i++) {
            tiny_string_buffer[i] = data[1 + i];
        }
        tiny_string_buffer[len] = '\0';
        return tiny_string_buffer;
    } else {
        String* s = as_string(v);
        return s->data;
    }
}

// Get string length in BYTES
static inline int string_lengthB(Value v) {
    if (!is_string(v)) return 0;
    
    if (is_tiny_string(v)) {
        const char* data = get_value_data_ptr_const(&v);
        return (int)(unsigned char)data[0];  // Byte length from first byte
    } else {
        String* s = as_string(v);
        return s->lenB;  // Byte length
    }
}

// Get string length in CHARACTERS (Unicode-aware)
static inline int string_length(Value v) {
    if (!is_string(v)) return 0;
    
    int lenB = string_lengthB(v);
    if (lenB == 0) return 0;
    
    if (is_tiny_string(v)) {
        const char* data = get_value_data_ptr_const(&v);
        return UTF8CharacterCount((const unsigned char*)(data + 1), lenB);
    } else {
        String* s = as_string(v);
        return UTF8CharacterCount((const unsigned char*)s->data, lenB);
    }
}

// Helper for TRUE zero-copy access to string data (no copying for tiny strings!)
// For scanning/comparing operations where null termination isn't needed
static inline const char* get_string_data_zerocopy(const Value* v_ptr, int* out_len) {
    Value v = *v_ptr;
    if (is_tiny_string(v)) {
        const char* data = get_value_data_ptr_const(v_ptr);
        *out_len = (int)(unsigned char)data[0];
        return data + 1;  // Return pointer directly to string data (no copying!)
    } else if (is_heap_string(v)) {
        String* s = (String*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
        *out_len = s->lenB;  // Fixed: use lenB instead of len
        return s->data;
    }
    *out_len = 0;
    return NULL;
}

// Helper for when we need null-terminated strings (optimized for tiny strings)
static inline const char* get_string_data_nullterm(const Value* v_ptr, char* tiny_buffer) {
    Value v = *v_ptr;
    if (is_tiny_string(v)) {
        const char* data = get_value_data_ptr_const(v_ptr);
        int len = (int)(unsigned char)data[0];
        
        // Optimization: if len < TINY_STRING_MAX_LEN, the data is already null-terminated
        // due to our invariant that unused bytes are zero
        if (len < TINY_STRING_MAX_LEN) {
            return data + 1;  // Return pointer directly into Value - already null-terminated!
        }
        
        // Only copy if string uses all 5 characters (no guaranteed null terminator)
        for (int i = 0; i < len; i++) {
            tiny_buffer[i] = data[1 + i];
        }
        tiny_buffer[len] = '\0';
        return tiny_buffer;
    } else if (is_heap_string(v)) {
        String* s = (String*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
        return s->data;
    }
    return NULL;
}

// String operations
static inline bool string_equals(Value a, Value b) {
    if (!is_string(a) || !is_string(b)) return false;
    
    // Fast path: both tiny strings - compare entire Values directly
    if (is_tiny_string(a) && is_tiny_string(b)) {
        // Since both have identical type masks and our invariant guarantees
        // unused bytes are zero, we can compare the entire Value
        return a == b;
    }
    
    // Mixed or heap strings: use zero-copy comparison
    int len_a, len_b;
    const char* str_a = get_string_data_zerocopy(&a, &len_a);
    const char* str_b = get_string_data_zerocopy(&b, &len_b);
    
    if (len_a != len_b) return false;
    return memcmp(str_a, str_b, len_a) == 0;  // Length-aware comparison, no null termination needed!
}

// String concatenation - implemented in nanbox_strings.c
Value string_concat(Value a, Value b);

// Find needle in haystack, return CHARACTER index (not byte index)
static inline int string_indexOf(Value haystack, Value needle) {
    // For strstr we still need null-terminated strings, so use the nullterm version
    char tiny_buffer_h[TINY_STRING_MAX_LEN + 1];
    char tiny_buffer_n[TINY_STRING_MAX_LEN + 1];
    
    const char* h = get_string_data_nullterm(&haystack, tiny_buffer_h);
    const char* n = get_string_data_nullterm(&needle, tiny_buffer_n);
    
    if (!h || !n) return -1;
    
    char* pos = strstr(h, n);
    if (pos == NULL) return -1;
    
    // Convert byte offset to character index
    int byteIndex = pos - h;
    int maxBytes = string_lengthB(haystack);
    return UTF8ByteIndexToCharIndex((const unsigned char*)h, byteIndex, maxBytes);
}

// Find needle in haystack, return BYTE index
static inline int string_indexOfB(Value haystack, Value needle) {
    char tiny_buffer_h[TINY_STRING_MAX_LEN + 1];
    char tiny_buffer_n[TINY_STRING_MAX_LEN + 1];
    
    const char* h = get_string_data_nullterm(&haystack, tiny_buffer_h);
    const char* n = get_string_data_nullterm(&needle, tiny_buffer_n);
    
    if (!h || !n) return -1;
    
    char* pos = strstr(h, n);
    return (pos == NULL) ? -1 : (pos - h);
}

// String replacement - implemented in nanbox_strings.c
Value string_replace(Value str, Value from, Value to);

// List structure 
typedef struct {
    int count;
    int capacity;
    Value items[];
} List;

static inline Value make_list(int capacity) {
    if (capacity <= 0) capacity = 8; // Default capacity
    List* list = (List*)gc_allocate(sizeof(List) + capacity * sizeof(Value));
    list->count = 0;
    list->capacity = capacity;
    return LIST_MASK | ((uintptr_t)list & 0xFFFFFFFFFFFFULL);
}

static inline List* as_list(Value v) {
    if (!is_list(v)) return NULL;
    return (List*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
}

static inline void list_add(Value list_val, Value item) {
    List* list = as_list(list_val);
    if (list && list->count < list->capacity) {
        list->items[list->count++] = item;
    }
}

static inline Value list_get(Value list_val, int index) {
    List* list = as_list(list_val);
    if (list && index >= 0 && index < list->count) {
        return list->items[index];
    }
    return make_nil();
}

static inline void list_set(Value list_val, int index, Value item) {
    List* list = as_list(list_val);
    if (list && index >= 0 && index < list->count) {
        list->items[index] = item;
    }
}

static inline int list_count(Value list_val) {
    List* list = as_list(list_val);
    return list ? list->count : 0;
}

static inline bool is_truthy(Value v) {
    if (is_nil(v)) return false;
    if (is_number(v)) return as_number(v) != 0.0;
    if (is_int(v)) return as_int(v) != 0;
    return true;
}

static inline bool values_equal(Value a, Value b) {
    if (is_number(a) && is_number(b)) {
        return as_number(a) == as_number(b);
    }
    if (is_string(a) && is_string(b)) {
        return string_equals(a, b);
    }
    return a == b;
}

static inline int list_indexOf(Value list_val, Value item) {
    List* list = as_list(list_val);
    if (!list) return -1;
    
    for (int i = 0; i < list->count; i++) {
        if (is_string(list->items[i]) && is_string(item)) {
            if (string_equals(list->items[i], item)) return i;
        } else if (values_equal(list->items[i], item)) {
            return i;
        }
    }
    return -1;
}

// String splitting - implemented in nanbox_strings.c
Value string_split(Value str, Value delimiter);

// Unicode-aware substring and character functions - implemented in nanbox_strings.c
Value string_substring(Value str, int startIndex, int len);
Value string_charAt(Value str, int index);

#endif
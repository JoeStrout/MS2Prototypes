#ifndef NANBOX_H
#define NANBOX_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

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

static inline bool is_null(Value v) {
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

static inline Value make_null() {
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
typedef struct {
    int len;
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
    if (str == NULL) return make_null();
    int len = strlen(str);
    
    // Use tiny string for short strings
    if (len <= TINY_STRING_MAX_LEN) {
        return make_tiny_string(str, len);
    }
    
    // Use heap string for longer strings
    String* s = (String*)gc_allocate(sizeof(String) + len + 1);
    s->len = len;
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

static inline int string_length(Value v) {
    if (!is_string(v)) return 0;
    
    if (is_tiny_string(v)) {
        const char* data = get_value_data_ptr_const(&v);
        return (int)(unsigned char)data[0];  // Length from first byte
    } else {
        String* s = as_string(v);
        return s->len;
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
        *out_len = s->len;
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

static inline Value string_concat(Value a, Value b) {
    int len_a = string_length(a);
    int len_b = string_length(b);
    int total_len = len_a + len_b;
    
    // Use TRUE zero-copy access - no copying for tiny strings!
    int actual_len_a, actual_len_b;
    const char* sa = get_string_data_zerocopy(&a, &actual_len_a);
    const char* sb = get_string_data_zerocopy(&b, &actual_len_b);
    
    if (!sa || !sb) return make_null();
    
    // Use tiny string if result is small enough
    if (total_len <= TINY_STRING_MAX_LEN) {
        char result_buffer[TINY_STRING_MAX_LEN + 1];
        // Direct memcpy from the in-Value data - no intermediate copying!
        memcpy(result_buffer, sa, actual_len_a);
        memcpy(result_buffer + actual_len_a, sb, actual_len_b);
        result_buffer[total_len] = '\0';
        return make_tiny_string(result_buffer, total_len);
    } else {
        // Use heap string for longer results
        String* result = (String*)gc_allocate(sizeof(String) + total_len + 1);
        result->len = total_len;
        memcpy(result->data, sa, actual_len_a);
        memcpy(result->data + actual_len_a, sb, actual_len_b);
        result->data[total_len] = '\0';
        return STRING_MASK | ((uintptr_t)result & 0xFFFFFFFFFFFFULL);
    }
}

static inline int string_indexOf(Value haystack, Value needle) {
    // For strstr we still need null-terminated strings, so use the nullterm version
    char tiny_buffer_h[TINY_STRING_MAX_LEN + 1];
    char tiny_buffer_n[TINY_STRING_MAX_LEN + 1];
    
    const char* h = get_string_data_nullterm(&haystack, tiny_buffer_h);
    const char* n = get_string_data_nullterm(&needle, tiny_buffer_n);
    
    if (!h || !n) return -1;
    
    char* pos = strstr(h, n);
    return (pos == NULL) ? -1 : (pos - h);
}

static inline Value string_replace(Value str, Value from, Value to) {
    int from_len = string_length(from);
    int to_len = string_length(to);
    int str_len = string_length(str);
    
    if (from_len == 0) return str; // Can't replace empty string
    
    // Get direct access to string data - use nullterm version for string operations
    char tiny_buffer_s[TINY_STRING_MAX_LEN + 1];
    char tiny_buffer_f[TINY_STRING_MAX_LEN + 1];  
    char tiny_buffer_t[TINY_STRING_MAX_LEN + 1];
    
    const char* s = get_string_data_nullterm(&str, tiny_buffer_s);
    const char* f = get_string_data_nullterm(&from, tiny_buffer_f);
    const char* t = get_string_data_nullterm(&to, tiny_buffer_t);
    
    if (!s || !f || !t) return make_null();
    
    // Count occurrences to calculate final length
    int count = 0;
    const char* temp = s;
    while ((temp = strstr(temp, f)) != NULL) {
        count++;
        temp += from_len;
    }
    
    if (count == 0) return str; // Return original if not found
    
    // Calculate new length
    int new_len = str_len + count * (to_len - from_len);
    
    // Use static buffer for small results, heap for larger ones
    char static_buffer[256]; // For small results
    char* temp_result;
    bool use_static = (new_len < sizeof(static_buffer));
    
    if (use_static) {
        temp_result = static_buffer;
    } else {
        temp_result = malloc(new_len + 1);
    }
    
    // Build the result string
    char* dest = temp_result;
    const char* src = s;
    
    while (*src) {
        const char* found = strstr(src, f);
        if (found == NULL) {
            // No more occurrences, copy the rest
            strcpy(dest, src);
            break;
        }
        
        // Copy text before the match
        int before_len = found - src;
        strncpy(dest, src, before_len);
        dest += before_len;
        
        // Copy replacement text
        strcpy(dest, t);
        dest += to_len;
        
        // Move source pointer past the match
        src = found + from_len;
    }
    
    // Create string using make_string (will use tiny string if small enough)
    Value result = make_string(temp_result);
    
    // Clean up heap allocation if used
    if (!use_static) {
        free(temp_result);
    }
    
    return result;
}

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
    return make_null();
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
    if (is_null(v)) return false;
    if (is_number(v)) return as_number(v) != 0.0;
    if (is_int(v)) return as_int(v) != 0;
    return true;
}

static inline bool value_equal(Value a, Value b) {
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
        } else if (value_equal(list->items[i], item)) {
            return i;
        }
    }
    return -1;
}

static inline Value string_split(Value str, Value delimiter) {
    int str_len = string_length(str);
    int delim_len = string_length(delimiter);
    
    // Get direct access to string data - use nullterm version for string operations
    char tiny_buffer_s[TINY_STRING_MAX_LEN + 1];
    char tiny_buffer_delim[TINY_STRING_MAX_LEN + 1];
    
    const char* s = get_string_data_nullterm(&str, tiny_buffer_s);
    if (!s) return make_null();
    
    const char* delim = NULL;
    if (delim_len > 0) {
        delim = get_string_data_nullterm(&delimiter, tiny_buffer_delim);
        if (!delim) return make_null();
    }
    
    // Handle empty delimiter (split into characters)
    if (!delim || strlen(delim) == 0) {
        Value list = make_list(str_len);
        for (int i = 0; i < str_len; i++) {
            char c[2] = {s[i], '\0'};
            list_add(list, make_string(c));
        }
        return list;
    }
    
    // Handle space delimiter as special case - manual split to preserve empty tokens
    if (strcmp(delim, " ") == 0) {
        Value list = make_list(100); // Rough estimate
        int start = 0;
        
        for (int i = 0; i <= str_len; i++) {
            if (i == str_len || s[i] == ' ') {
                // Found delimiter or end of string
                int token_len = i - start;
                char token_buffer[256];
                if (token_len < sizeof(token_buffer)) {
                    strncpy(token_buffer, s + start, token_len);
                    token_buffer[token_len] = '\0';
                    list_add(list, make_string(token_buffer));
                } else {
                    char* token = malloc(token_len + 1);
                    strncpy(token, s + start, token_len);
                    token[token_len] = '\0';
                    list_add(list, make_string(token));
                    free(token);
                }
                start = i + 1;
            }
        }
        return list;
    }
    
    // General case: split on specific delimiter
    // Need to copy for strtok since it modifies the string
    Value list = make_list(50); // Rough estimate
    char s_buffer[256];
    char* s_copy;
    bool use_static = (str_len < sizeof(s_buffer));
    
    if (use_static) {
        strcpy(s_buffer, s);
        s_copy = s_buffer;
    } else {
        s_copy = malloc(str_len + 1);
        strcpy(s_copy, s);
    }
    
    char* token = strtok(s_copy, delim);
    while (token != NULL) {
        list_add(list, make_string(token));
        token = strtok(NULL, delim);
    }
    
    if (!use_static) {
        free(s_copy);
    }
    
    return list;
}

#endif
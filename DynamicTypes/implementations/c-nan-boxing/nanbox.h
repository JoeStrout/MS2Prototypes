#ifndef NANBOX_H
#define NANBOX_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

typedef uint64_t Value;

// Masks and constants from the article
#define NANISH_MASK        0xffff000000000000ULL
#define NANISH             0x7ffc000000000000ULL
#define INTEGER_MASK       0x7ffc000000000000ULL
#define OBJECT_MASK        0xfffc000000000000ULL
#define STRING_MASK        0xfffe000000000000ULL
#define LIST_MASK          0xfffd000000000000ULL

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

static inline bool is_string(Value v) {
    return (v & NANISH_MASK) == STRING_MASK;
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
typedef struct {
    int len;
    char data[];
} String;

static inline Value make_string(const char* str) {
    if (str == NULL) return make_nil();
    int len = strlen(str);
    String* s = malloc(sizeof(String) + len + 1);
    s->len = len;
    strcpy(s->data, str);
    return STRING_MASK | ((uintptr_t)s & 0xFFFFFFFFFFFFULL);
}

static inline String* as_string(Value v) {
    return (String*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
}

static inline const char* as_cstring(Value v) {
    if (!is_string(v)) return NULL;
    String* s = as_string(v);
    return s->data;
}

static inline int string_length(Value v) {
    if (!is_string(v)) return 0;
    String* s = as_string(v);
    return s->len;
}

// String operations
static inline bool string_equals(Value a, Value b) {
    if (!is_string(a) || !is_string(b)) return false;
    String* sa = as_string(a);
    String* sb = as_string(b);
    if (sa->len != sb->len) return false;
    return strcmp(sa->data, sb->data) == 0;
}

static inline Value string_concat(Value a, Value b) {
    const char* sa = as_cstring(a);
    const char* sb = as_cstring(b);
    if (!sa || !sb) return make_nil();
    
    int len_a = string_length(a);
    int len_b = string_length(b);
    int total_len = len_a + len_b;
    
    String* result = malloc(sizeof(String) + total_len + 1);
    result->len = total_len;
    strcpy(result->data, sa);
    strcat(result->data, sb);
    
    return STRING_MASK | ((uintptr_t)result & 0xFFFFFFFFFFFFULL);
}

static inline int string_indexOf(Value haystack, Value needle) {
    const char* h = as_cstring(haystack);
    const char* n = as_cstring(needle);
    if (!h || !n) return -1;
    
    char* pos = strstr(h, n);
    if (pos == NULL) return -1;
    return pos - h;
}

static inline Value string_replace(Value str, Value from, Value to) {
    const char* s = as_cstring(str);
    const char* f = as_cstring(from);
    const char* t = as_cstring(to);
    if (!s || !f || !t) return make_nil();
    
    int from_len = string_length(from);
    int to_len = string_length(to);
    int str_len = string_length(str);
    
    if (from_len == 0) return str; // Can't replace empty string
    
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
    
    String* result = malloc(sizeof(String) + new_len + 1);
    result->len = new_len;
    
    // Build the result string
    char* dest = result->data;
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
    
    return STRING_MASK | ((uintptr_t)result & 0xFFFFFFFFFFFFULL);
}

// List structure 
typedef struct {
    int count;
    int capacity;
    Value items[];
} List;

static inline Value make_list(int capacity) {
    if (capacity <= 0) capacity = 8; // Default capacity
    List* list = malloc(sizeof(List) + capacity * sizeof(Value));
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

static inline Value string_split(Value str, Value delimiter) {
    const char* s = as_cstring(str);
    const char* delim = as_cstring(delimiter);
    if (!s) return make_nil();
    
    int str_len = string_length(str);
    
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
                char* token = malloc(token_len + 1);
                strncpy(token, s + start, token_len);
                token[token_len] = '\0';
                list_add(list, make_string(token));
                free(token);
                start = i + 1;
            }
        }
        return list;
    }
    
    // General case: split on specific delimiter
    Value list = make_list(50); // Rough estimate
    char* s_copy = malloc(str_len + 1);
    strcpy(s_copy, s);
    
    char* token = strtok(s_copy, delim);
    while (token != NULL) {
        list_add(list, make_string(token));
        token = strtok(NULL, delim);
    }
    free(s_copy);
    return list;
}

#endif
#ifndef NANBOX_GC_H
#define NANBOX_GC_H

// This header combines nanbox and gc functionality properly

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

// #define GC_DEBUG 1
// #define GC_AGGRESSIVE 1  // Collect on every allocation (for testing)

typedef uint64_t Value;

// Masks and constants from the article
#define NANISH_MASK        0xffff000000000000ULL
#define NANISH             0x7ffc000000000000ULL
#define INTEGER_MASK       0x7ffc000000000000ULL
#define OBJECT_MASK        0xfffc000000000000ULL
#define STRING_MASK        0xfffe000000000000ULL
#define LIST_MASK          0xfffd000000000000ULL

#define NULL_VALUE         0x7ffe000000000000ULL

// String and List structures
typedef struct {
    int len;
    char data[];
} String;

typedef struct {
    int count;
    int capacity;
    Value items[];
} List;

// GC Object header - minimal overhead
typedef struct GCObject {
    struct GCObject* next;  // Linked list of all objects
    bool marked;           // Mark bit for GC
    size_t size;          // Size for sweep phase
} GCObject;

// Scope management for RAII-style protection
typedef struct GCScope {
    int start_index;      // Where this scope starts in the root stack
} GCScope;

// Root set management - shadow stack of pointers to local Values
typedef struct GCRootSet {
    Value** roots;        // Array of pointers to Values (shadow stack)
    int count;
    int capacity;
} GCRootSet;

// GC state
typedef struct GC {
    GCObject* all_objects;    // Linked list of all allocated objects
    GCRootSet root_set;       // Stack of root values
    GCScope scope_stack[64];  // Stack of scopes for RAII-style protection
    int scope_count;          // Number of active scopes
    size_t bytes_allocated;   // Total allocated memory
    size_t gc_threshold;      // Trigger collection when exceeded
    int disable_count;        // Counter for nested disable/enable calls
} GC;

// Global GC instance
extern GC gc;

// GC lifecycle functions
void gc_init(void);
void gc_shutdown(void);
void gc_collect(void);

// Root set management (shadow stack approach)
void gc_protect_value(Value* val_ptr);
void gc_unprotect_value(void);

// Scope management
void gc_push_scope(void);
void gc_pop_scope(void);

// Critical section management
void gc_disable(void);
void gc_enable(void);

// Object allocation
void* gc_allocate(size_t size);

// Mark functions for different object types
void gc_mark_value(Value v);
void gc_mark_string(String* str);
void gc_mark_list(List* list);

// Utility macros for root management
#define GC_PROTECT(val_ptr) gc_protect_value(val_ptr)
#define GC_UNPROTECT() gc_unprotect_value()

// Scope-based protection macros
#define GC_PUSH_SCOPE() gc_push_scope()
#define GC_POP_SCOPE() gc_pop_scope()

// SHADOW STACK PROTECTION STANDARD:
// 1. Call GC_PUSH_SCOPE() at start of function
// 2. GC_PROTECT(&local_var) for all local Value variables (pass pointer!)
// 3. Call GC_POP_SCOPE() before return
// 4. Collection only happens at safe points (allocation, explicit gc_collect)
// 5. When reassigning protected values, the shadow stack automatically tracks them
//
// Example usage:
//   void my_function() {
//       GC_PUSH_SCOPE();
//       Value str = make_nil();
//       Value list = make_nil();
//       GC_PROTECT(&str);    // Protect pointer to str
//       GC_PROTECT(&list);   // Protect pointer to list
//       
//       str = make_string("hello");  // Reassignment is fine
//       list = make_list(10);        // Shadow stack sees new values
//       
//       GC_POP_SCOPE();
//   }

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

static inline Value make_string(const char* str) {
    if (str == NULL) return make_nil();
    int len = strlen(str);
    String* s = (String*)gc_allocate(sizeof(String) + len + 1);
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
        String* sa = as_string(a);
        String* sb = as_string(b);
        if (sa->len != sb->len) return false;
        return strcmp(sa->data, sb->data) == 0;
    }
    return a == b;
}

static inline bool string_equals(Value a, Value b) {
    if (!is_string(a) || !is_string(b)) return false;
    String* sa = as_string(a);
    String* sb = as_string(b);
    if (sa->len != sb->len) return false;
    return strcmp(sa->data, sb->data) == 0;
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

static inline Value string_concat(Value a, Value b) {
    const char* sa = as_cstring(a);
    const char* sb = as_cstring(b);
    if (!sa || !sb) return make_nil();
    
    int len_a = string_length(a);
    int len_b = string_length(b);
    int total_len = len_a + len_b;
    
    String* result = (String*)gc_allocate(sizeof(String) + total_len + 1);
    result->len = total_len;
    strcpy(result->data, sa);
    strcat(result->data, sb);
    
    return STRING_MASK | ((uintptr_t)result & 0xFFFFFFFFFFFFULL);
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
    
    String* result = (String*)gc_allocate(sizeof(String) + new_len + 1);
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
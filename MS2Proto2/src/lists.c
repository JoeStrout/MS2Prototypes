// lists.c
//
// List implementation for NaN-boxed Values
// Dynamic arrays with automatic capacity management

#include "../include/lists.h"
#include "../include/nanbox.h"
#include "../include/gc.h"
#include "../include/strings.h"
#include <stdlib.h>
#include <assert.h>

// List creation and management
Value make_list(int initial_capacity) {
    if (initial_capacity <= 0) initial_capacity = 8; // Default capacity
    List* list = (List*)gc_allocate(sizeof(List) + initial_capacity * sizeof(Value));
    list->count = 0;
    list->capacity = initial_capacity;
    return LIST_MASK | ((uintptr_t)list & 0xFFFFFFFFFFFFULL);
}

Value make_empty_list(void) {
    return make_list(8);  // Default capacity
}

// List access
List* as_list(Value v) {
    if (!is_list(v)) return NULL;
    return (List*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
}

int list_count(Value list_val) {
    List* list = as_list(list_val);
    return list ? list->count : 0;
}

int list_capacity(Value list_val) {
    List* list = as_list(list_val);
    return list ? list->capacity : 0;
}

// List element operations
Value list_get(Value list_val, int index) {
    List* list = as_list(list_val);
    if (list && index >= 0 && index < list->count) {
        return list->items[index];
    }
    return make_nil();
}

void list_set(Value list_val, int index, Value item) {
    List* list = as_list(list_val);
    if (list && index >= 0 && index < list->count) {
        list->items[index] = item;
    }
}

void list_push(Value list_val, Value item) {
    List* list = as_list(list_val);
    if (!list) return;
    
    // TODO: Implement capacity expansion if needed
    if (list->count < list->capacity) {
        list->items[list->count++] = item;
    }
}

Value list_pop(Value list_val) {
    List* list = as_list(list_val);
    if (!list || list->count <= 0) return make_nil();
    
    return list->items[--list->count];
}

void list_insert(Value list_val, int index, Value item) {
    List* list = as_list(list_val);
    if (!list || index < 0 || index > list->count) return;
    
    // TODO: Implement capacity expansion if needed
    if (list->count >= list->capacity) return;
    
    // Shift elements to the right
    for (int i = list->count; i > index; i--) {
        list->items[i] = list->items[i-1];
    }
    
    list->items[index] = item;
    list->count++;
}

void list_remove(Value list_val, int index) {
    List* list = as_list(list_val);
    if (!list || index < 0 || index >= list->count) return;
    
    // Shift elements to the left
    for (int i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i+1];
    }
    
    list->count--;
}

// Value equality helper
static bool values_equal(Value a, Value b) {
    if (is_number(a) && is_number(b)) {
        return as_double(a) == as_double(b);
    }
    if (is_int(a) && is_int(b)) {
        return as_int(a) == as_int(b);
    }
    if (is_string(a) && is_string(b)) {
        return string_equals(a, b);
    }
    return a == b;
}

// List searching
int list_indexOf(Value list_val, Value item, int start_pos) {
    List* list = as_list(list_val);
    if (!list) return -1;
    
    if (start_pos < 0) start_pos = 0;
    
    for (int i = start_pos; i < list->count; i++) {
        if (values_equal(list->items[i], item)) {
            return i;
        }
    }
    return -1;
}

bool list_contains(Value list_val, Value item) {
    return list_indexOf(list_val, item, 0) != -1;
}

// List utilities
void list_clear(Value list_val) {
    List* list = as_list(list_val);
    if (list) {
        list->count = 0;
    }
}

Value list_copy(Value list_val) {
    List* src = as_list(list_val);
    if (!src) return make_nil();
    
    Value new_list = make_list(src->capacity);
    List* dst = as_list(new_list);
    
    dst->count = src->count;
    for (int i = 0; i < src->count; i++) {
        dst->items[i] = src->items[i];
    }
    
    return new_list;
}

void list_resize(Value list_val, int new_capacity) {
    // TODO: Implement list resizing
    // This would require creating a new list and copying elements
    // For now, this is a stub
    (void)list_val;
    (void)new_capacity;
}
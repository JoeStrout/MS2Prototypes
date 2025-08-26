#ifndef LISTS_H
#define LISTS_H

#include "nanbox.h"
#include <stdbool.h>

// List implementation for NaN-boxed Values
// Dynamic arrays with automatic capacity management

// List structure
typedef struct {
    int count;       // Number of elements
    int capacity;    // Allocated capacity
    Value items[];   // Array of Values
} List;

// List creation and management
Value make_list(int initial_capacity);
Value make_empty_list(void);

// List access
List* as_list(Value v);
int list_count(Value list_val);
int list_capacity(Value list_val);

// List element operations
Value list_get(Value list_val, int index);
void list_set(Value list_val, int index, Value item);
void list_push(Value list_val, Value item);
Value list_pop(Value list_val);
void list_insert(Value list_val, int index, Value item);
void list_remove(Value list_val, int index);

// List searching
int list_indexOf(Value list_val, Value item, int start_pos);
bool list_contains(Value list_val, Value item);

// List utilities
void list_clear(Value list_val);
Value list_copy(Value list_val);
void list_resize(Value list_val, int new_capacity);

#endif // LISTS_H
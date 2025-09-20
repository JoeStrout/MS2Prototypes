// Map implementation for NaN-boxed Values. I.e., this is the underlying
// implementation of MiniScript maps (dictionaries). All memory management
// is done via the gc module.

#ifndef VALUE_MAP_H
#define VALUE_MAP_H

#include "value.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Map and MapEntry structures are defined in value.h

// Map creation and management
Value make_map(int initial_capacity);
Value make_empty_map(void);

// Map access
ValueMap* as_map(Value v);
int map_count(Value map_val);
int map_capacity(Value map_val);

// Map operations
Value map_get(Value map_val, Value key);
bool map_set(Value map_val, Value key, Value value);
bool map_remove(Value map_val, Value key);
bool map_has_key(Value map_val, Value key);

// Map utilities
void map_clear(Value map_val);
Value map_copy(Value map_val);
bool map_needs_expansion(Value map_val);
bool map_expand_capacity(Value map_val);  // Expands in-place, returns success
Value map_with_expanded_capacity(Value map_val);  // Deprecated - creates new map

// Map iteration
typedef struct {
    ValueMap* map;
    int index;
} MapIterator;

MapIterator map_iterator(Value map_val);
bool map_iterator_next(MapIterator* iter, Value* out_key, Value* out_value);

// Hash function for maps
uint32_t map_hash(Value map_val);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VALUE_MAP_H
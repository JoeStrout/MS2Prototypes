#include "value_map.h"
#include "value.h"
#include "gc.h"
#include "hashing.h"
#include <stdlib.h>
#include <assert.h>

// Default load factor threshold (resize when > 75% full)
#define LOAD_FACTOR_THRESHOLD 0.75

// Map creation and management
Value make_map(int initial_capacity) {
    if (initial_capacity <= 0) initial_capacity = 8; // Default capacity

    // Allocate the ValueMap structure
    ValueMap* map = (ValueMap*)gc_allocate(sizeof(ValueMap));
    map->count = 0;
    map->capacity = initial_capacity;

    // Allocate the entries array separately
    map->entries = (MapEntry*)gc_allocate(initial_capacity * sizeof(MapEntry));

    // Initialize all entries as unoccupied
    for (int i = 0; i < initial_capacity; i++) {
        map->entries[i].occupied = false;
        map->entries[i].key = make_null();
        map->entries[i].value = make_null();
        map->entries[i].hash = 0;
    }

    return MAP_MASK | ((uintptr_t)map & 0xFFFFFFFFFFFFULL);
}

Value make_empty_map(void) {
    return make_map(8);  // Default capacity
}

// Map access
ValueMap* as_map(Value v) {
    if (!is_map(v)) return NULL;
    return (ValueMap*)(uintptr_t)(v & 0xFFFFFFFFFFFFULL);
}

int map_count(Value map_val) {
    ValueMap* map = as_map(map_val);
    return map ? map->count : 0;
}

int map_capacity(Value map_val) {
    ValueMap* map = as_map(map_val);
    return map ? map->capacity : 0;
}

// Find entry by key using linear probing
static int find_entry(ValueMap* map, Value key, uint32_t hash) {
    if (!map || map->capacity == 0) return -1;

    int index = hash % map->capacity;
    int original_index = index;

    do {
        MapEntry* entry = &map->entries[index];

        if (!entry->occupied) {
            // Found empty slot
            return index;
        }

        if (entry->hash == hash && value_equal(entry->key, key)) {
            // Found matching key
            return index;
        }

        // Linear probing
        index = (index + 1) % map->capacity;
    } while (index != original_index);

    // Table is full
    return -1;
}

// Map operations
Value map_get(Value map_val, Value key) {
    ValueMap* map = as_map(map_val);
    if (!map) return make_null();

    uint32_t hash = value_hash(key);
    int index = find_entry(map, key, hash);

    if (index >= 0 && map->entries[index].occupied) {
        return map->entries[index].value;
    }

    return make_null();
}

bool map_set(Value map_val, Value key, Value value) {
    ValueMap* map = as_map(map_val);
    if (!map) return false;

    // Check if we need to expand before adding
    if (map_needs_expansion(map_val)) {
        if (!map_expand_capacity(map_val)) {
            return false; // Expansion failed
        }
    }

    uint32_t hash = value_hash(key);
    int index = find_entry(map, key, hash);

    if (index < 0) {
        // Table is full - this shouldn't happen if we expand properly
        return false;
    }

    MapEntry* entry = &map->entries[index];

    if (!entry->occupied) {
        // New entry
        entry->key = key;
        entry->hash = hash;
        entry->occupied = true;
        map->count++;
    }

    // Set or update value
    entry->value = value;
    return true;
}

bool map_remove(Value map_val, Value key) {
    ValueMap* map = as_map(map_val);
    if (!map) return false;

    uint32_t hash = value_hash(key);
    int index = find_entry(map, key, hash);

    if (index >= 0 && map->entries[index].occupied) {
        MapEntry* entry = &map->entries[index];
        entry->occupied = false;
        entry->key = make_null();
        entry->value = make_null();
        entry->hash = 0;
        map->count--;

        // Rehash entries that might have been displaced by linear probing
        int rehash_index = (index + 1) % map->capacity;
        while (rehash_index != index && map->entries[rehash_index].occupied) {
            MapEntry temp = map->entries[rehash_index];
            map->entries[rehash_index].occupied = false;
            map->count--;

            // Re-insert the displaced entry
            map_set(map_val, temp.key, temp.value);

            rehash_index = (rehash_index + 1) % map->capacity;
        }

        return true;
    }

    return false;
}

bool map_has_key(Value map_val, Value key) {
    ValueMap* map = as_map(map_val);
    if (!map) return false;

    uint32_t hash = value_hash(key);
    int index = find_entry(map, key, hash);

    return index >= 0 && map->entries[index].occupied;
}

// Map utilities
void map_clear(Value map_val) {
    ValueMap* map = as_map(map_val);
    if (!map) return;

    for (int i = 0; i < map->capacity; i++) {
        map->entries[i].occupied = false;
        map->entries[i].key = make_null();
        map->entries[i].value = make_null();
        map->entries[i].hash = 0;
    }
    map->count = 0;
}

Value map_copy(Value map_val) {
    ValueMap* src_map = as_map(map_val);
    if (!src_map) return make_empty_map();

    Value new_map = make_map(src_map->capacity);

    for (int i = 0; i < src_map->capacity; i++) {
        if (src_map->entries[i].occupied) {
            map_set(new_map, src_map->entries[i].key, src_map->entries[i].value);
        }
    }

    return new_map;
}

bool map_needs_expansion(Value map_val) {
    ValueMap* map = as_map(map_val);
    if (!map) return false;

    double load_factor = (double)map->count / (double)map->capacity;
    return load_factor > LOAD_FACTOR_THRESHOLD;
}

// Expand map capacity in-place (preserves the Value/MemRef)
bool map_expand_capacity(Value map_val) {
    ValueMap* map = as_map(map_val);
    if (!map) return false;

    int old_capacity = map->capacity;
    int new_capacity = old_capacity * 2;

    // Save old entries
    MapEntry* old_entries = map->entries;

    // Allocate new entries array
    MapEntry* new_entries = (MapEntry*)gc_allocate(new_capacity * sizeof(MapEntry));
    if (!new_entries) return false;

    // Initialize new entries
    for (int i = 0; i < new_capacity; i++) {
        new_entries[i].occupied = false;
        new_entries[i].key = make_null();
        new_entries[i].value = make_null();
        new_entries[i].hash = 0;
    }

    // Update map structure
    map->entries = new_entries;
    map->capacity = new_capacity;
    map->count = 0; // Will be rebuilt as we re-insert

    // Re-insert all entries from old array
    for (int i = 0; i < old_capacity; i++) {
        if (old_entries[i].occupied) {
            // Use internal insertion logic to avoid recursion
            uint32_t hash = old_entries[i].hash;
            int index = find_entry(map, old_entries[i].key, hash);
            if (index >= 0) {
                map->entries[index].key = old_entries[i].key;
                map->entries[index].value = old_entries[i].value;
                map->entries[index].hash = hash;
                map->entries[index].occupied = true;
                map->count++;
            }
        }
    }

    // Note: old_entries will be garbage collected since it's no longer referenced
    return true;
}

// Deprecated: Creates a new map instead of expanding in-place
Value map_with_expanded_capacity(Value map_val) {
    ValueMap* old_map = as_map(map_val);
    if (!old_map) return map_val;

    int new_capacity = old_map->capacity * 2;
    Value new_map = make_map(new_capacity);

    // Copy all entries to the new map
    for (int i = 0; i < old_map->capacity; i++) {
        if (old_map->entries[i].occupied) {
            map_set(new_map, old_map->entries[i].key, old_map->entries[i].value);
        }
    }

    return new_map;
}

// Map iteration
MapIterator map_iterator(Value map_val) {
    MapIterator iter;
    iter.map = as_map(map_val);
    iter.index = -1;
    return iter;
}

bool map_iterator_next(MapIterator* iter, Value* out_key, Value* out_value) {
    if (!iter || !iter->map) return false;

    // Find next occupied entry
    iter->index++;
    while (iter->index < iter->map->capacity) {
        if (iter->map->entries[iter->index].occupied) {
            if (out_key) *out_key = iter->map->entries[iter->index].key;
            if (out_value) *out_value = iter->map->entries[iter->index].value;
            return true;
        }
        iter->index++;
    }

    return false;
}

// Hash function for maps
uint32_t map_hash(Value map_val) {
    ValueMap* map = as_map(map_val);
    if (!map) return 0;

    // Hash based on all key-value pairs
    // Use FNV-1a constants for consistency
    const uint32_t FNV_PRIME = 0x01000193;
    uint32_t hash = 0x811c9dc5; // FNV-1a offset basis

    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i].occupied) {
            // Combine key and value hashes
            uint32_t key_hash = value_hash(map->entries[i].key);
            uint32_t value_hash_val = value_hash(map->entries[i].value);

            // XOR key and value hashes, then combine with running hash
            uint32_t pair_hash = key_hash ^ value_hash_val;
            hash ^= pair_hash;
            hash *= FNV_PRIME;
        }
    }

    // Ensure hash is never 0 (reserved for "not computed")
    return hash == 0 ? 1 : hash;
}

// Convert map to string representation for runtime (returns GC-managed Value)
Value map_to_string(Value map_val) {
    ValueMap* map = as_map(map_val);
    if (!map) return make_string("{}");

    if (map->count == 0) return make_string("{}");

    // Build string: {"key1": "value1", "key2": "value2"}
    Value result = make_string("{");

    MapIterator iter = map_iterator(map_val);
    Value key, value;
    bool first = true;

	// ToDo: instead of repeatedly calling string_concat, gather all
	// the items in a list, and use join.
    while (map_iterator_next(&iter, &key, &value)) {
        if (!first) {
            Value comma = make_string(", ");
            result = string_concat(result, comma);
        }
        first = false;

        // Get string representation of key and value
        Value key_str = value_repr(key);
        Value value_str = value_repr(value);

        // Add "key": "value"
        result = string_concat(result, key_str);
        Value colon = make_string(": ");
        result = string_concat(result, colon);
        result = string_concat(result, value_str);
    }

    Value close = make_string("}");
    result = string_concat(result, close);

    return result;
}
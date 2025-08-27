#pragma once
#include <cstdint>

// Forward declaration
struct StringStorage;

// StringStorage allocator function type
// Takes source string, byte length, hash, and returns StringStorage*
// The returned StringStorage should have data copied and all fields set
typedef StringStorage* (*StringStorageAllocator)(const char* source, int byteLen, uint32_t hash);

// Default allocator - simple malloc-based allocation
StringStorage* defaultStringAllocator(const char* source, int byteLen, uint32_t hash);

// String intern pool functions
namespace StringPool {
    // Hash table entry for quick lookup
    struct HashEntry {
        uint32_t hash;
        uint16_t index;
        HashEntry* next;
    };
    
    // Pool storage: each pool contains an array of StringStorage pointers
    // and a hash table for quick lookup
    struct Pool {
        StringStorage** strings;
        uint16_t capacity;
        uint16_t count;
        HashEntry* hashTable[256];  // Small hash table
        bool initialized;
    };
    
    // Initialize a specific pool
    void initPool(uint8_t poolNum);
    
    // Find existing string in pool or add new one using allocator
    // Returns index within the pool
    uint16_t internString(uint8_t poolNum, const char* cstr, StringStorageAllocator allocator);
    
    // Get StringStorage pointer for a given pool and index
    StringStorage* getStorage(uint8_t poolNum, uint16_t index);
    
    // Get C string pointer for a given pool and index
    const char* getCString(uint8_t poolNum, uint16_t index);
    
    // Pool-aware allocator - checks for existing strings before allocating
    StringStorage* poolAwareAllocator(const char* source, int byteLen, uint32_t hash);
    
    // Set which pool the pool-aware allocator should use (default: 0)
    void setDefaultPool(uint8_t poolNum);
}
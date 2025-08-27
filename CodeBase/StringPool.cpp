#include "StringPool.h"
#include "string.h"  // For StringStorage definition
#include <cstdlib>
#include <cstring>
extern "C" {
#include "unicodeUtil.h"
}

// FNV-1a hash function
static uint32_t string_hash(const char* data, int len) {
    const uint32_t FNV_PRIME = 0x01000193;
    const uint32_t FNV_OFFSET_BASIS = 0x811c9dc5;
    
    uint32_t hash = FNV_OFFSET_BASIS;
    for (int i = 0; i < len; i++) {
        hash ^= (unsigned char)data[i];
        hash *= FNV_PRIME;
    }
    
    return hash == 0 ? 1 : hash;
}

// Simple UTF-8 character counter
static int utf8_char_count(const char* str, int byteLen) {
    int count = 0;
    for (int i = 0; i < byteLen; i++) {
        if ((str[i] & 0xC0) != 0x80) {
            count++;
        }
    }
    return count;
}

// Default allocator implementation
StringStorage* defaultStringAllocator(const char* source, int byteLen, uint32_t hash) {
    StringStorage* storage = (StringStorage*)malloc(sizeof(StringStorage) + byteLen + 1);
    if (!storage) return nullptr;
    
    storage->lenB = byteLen;
    storage->lenC = utf8_char_count(source, byteLen);
    storage->hash = hash;
    strcpy(storage->data, source);
    
    return storage;
}

namespace StringPool {
    static Pool pools[256];
    static uint8_t defaultPoolNum = 0;
    
    void initPool(uint8_t poolNum) {
        Pool* pool = &pools[poolNum];
        if (pool->initialized) return;
        
        pool->initialized = true;
        pool->capacity = 16;
        pool->count = 0;
        pool->strings = (StringStorage**)malloc(pool->capacity * sizeof(StringStorage*));
        
        // Initialize hash table
        for (int i = 0; i < 256; i++) {
            pool->hashTable[i] = nullptr;
        }
        
        // Index 0 always represents empty string
        StringStorage* empty = defaultStringAllocator("", 0, string_hash("", 0));
        
        pool->strings[0] = empty;
        pool->count = 1;
        
        // Add to hash table
        uint8_t bucket = empty->hash & 0xFF;
        HashEntry* entry = (HashEntry*)malloc(sizeof(HashEntry));
        entry->hash = empty->hash;
        entry->index = 0;
        entry->next = pool->hashTable[bucket];
        pool->hashTable[bucket] = entry;
    }
    
    uint16_t internString(uint8_t poolNum, const char* cstr, StringStorageAllocator allocator) {
        initPool(poolNum);
        Pool* pool = &pools[poolNum];
        
        int lenB = strlen(cstr);
        
        // Empty string is always at index 0
        if (lenB == 0) return 0;
        
        uint32_t hash = string_hash(cstr, lenB);
        uint8_t bucket = hash & 0xFF;
        
        // Check if string already exists
        HashEntry* entry = pool->hashTable[bucket];
        while (entry) {
            if (entry->hash == hash && entry->index < pool->count) {
                StringStorage* storage = pool->strings[entry->index];
                if (storage && storage->lenB == lenB && 
                    memcmp(storage->data, cstr, lenB) == 0) {
                    return entry->index;
                }
            }
            entry = entry->next;
        }
        
        // String not found, create new one
        StringStorage* storage = allocator(cstr, lenB, hash);
        if (!storage) return 0;
        
        // Expand array if needed
        if (pool->count >= pool->capacity) {
            pool->capacity *= 2;
            pool->strings = (StringStorage**)realloc(pool->strings, 
                pool->capacity * sizeof(StringStorage*));
        }
        
        uint16_t index = pool->count++;
        pool->strings[index] = storage;
        
        // Add to hash table
        HashEntry* newEntry = (HashEntry*)malloc(sizeof(HashEntry));
        newEntry->hash = hash;
        newEntry->index = index;
        newEntry->next = pool->hashTable[bucket];
        pool->hashTable[bucket] = newEntry;
        
        return index;
    }
    
    StringStorage* getStorage(uint8_t poolNum, uint16_t index) {
        Pool* pool = &pools[poolNum];
        if (!pool->initialized) return nullptr;
        if (index >= pool->count) return nullptr;
        return pool->strings[index];
    }
    
    const char* getCString(uint8_t poolNum, uint16_t index) {
        StringStorage* storage = getStorage(poolNum, index);
        return storage ? storage->data : "";
    }
    
    StringStorage* poolAwareAllocator(const char* source, int byteLen, uint32_t hash) {
        // First check if string already exists in default pool
        initPool(defaultPoolNum);
        Pool* pool = &pools[defaultPoolNum];
        
        uint8_t bucket = hash & 0xFF;
        HashEntry* entry = pool->hashTable[bucket];
        while (entry) {
            if (entry->hash == hash && entry->index < pool->count) {
                StringStorage* storage = pool->strings[entry->index];
                if (storage && storage->lenB == byteLen && 
                    memcmp(storage->data, source, byteLen) == 0) {
                    return storage; // Return existing storage
                }
            }
            entry = entry->next;
        }
        
        // Not found, create new one and add to pool
        StringStorage* storage = defaultStringAllocator(source, byteLen, hash);
        if (!storage) return nullptr;
        
        // Expand array if needed
        if (pool->count >= pool->capacity) {
            pool->capacity *= 2;
            pool->strings = (StringStorage**)realloc(pool->strings, 
                pool->capacity * sizeof(StringStorage*));
        }
        
        uint16_t index = pool->count++;
        pool->strings[index] = storage;
        
        // Add to hash table
        HashEntry* newEntry = (HashEntry*)malloc(sizeof(HashEntry));
        newEntry->hash = hash;
        newEntry->index = index;
        newEntry->next = pool->hashTable[bucket];
        pool->hashTable[bucket] = newEntry;
        
        return storage;
    }
    
    void setDefaultPool(uint8_t poolNum) {
        defaultPoolNum = poolNum;
    }
}
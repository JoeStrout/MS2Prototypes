// StringPool.h provides a string interning service, as well as a general
// StringStorage allocator, for the String class (i.e., host strings).  It
// is built on top of MemPool.

#pragma once
#include "MemPool.h"
#include "StringStorage.h"
#include <cstdint>

namespace StringPool {

struct HashEntry {
	uint32_t hash;
	uint16_t index; // into the pool's strings array
	MemRef   next;  // MemRef -> HashEntry
};

struct Pool {
	// MemRef to a growable array of MemRef (each points to a StringStorage)
	MemRef   stringsRef;          // MemRef -> MemRef[count] (array of StringStorage refs)
	uint16_t capacity;
	uint16_t count;

	// Hash table heads are MemRefs to HashEntry
	MemRef   hashTable[256];

	bool     initialized;
};

// — public API unchanged —
uint16_t internString(uint8_t poolNum, const char* cstr);
const char* getCString(uint8_t poolNum, uint16_t idx);
const StringStorage* getStorage(uint8_t poolNum, uint16_t idx);

StringStorage* defaultStringAllocator(const char* src, int lenB, uint32_t hash);

// Allocator function compatible with StringStorageAllocator typedef
// Uses pool 0 by default
void* poolAllocator(size_t size);

// Pool-specific allocator (can be used with std::bind or similar)
void* poolAllocatorForPool(size_t size, uint8_t poolNum);

} // namespace StringPool

#ifdef __cplusplus
extern "C" {
#endif

// C-compatible allocator function for use in C code
void* stringpool_allocator(size_t size);

#ifdef __cplusplus
}
#endif

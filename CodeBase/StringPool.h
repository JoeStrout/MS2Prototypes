// StringPool.h
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


} // namespace StringPool

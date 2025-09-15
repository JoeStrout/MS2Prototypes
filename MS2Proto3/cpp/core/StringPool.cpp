// StringPool.cpp
#include "StringPool.h"
#include <cstring>  // strlen, memcpy
#include <malloc/malloc.h> // for debugging (malloc_size) on macOS
#include <cstdio>

namespace StringPool {

static Pool pools[256];  // ToDo: parameterize this

// Small helpers to keep code tidy
static inline MemRef* stringsArray(Pool* p) {
    return (MemRef*)MemPoolManager::getPtr(p->stringsRef);
}
static inline HashEntry* derefHE(MemRef r) {
    return (HashEntry*)MemPoolManager::getPtr(r);
}
static inline StringStorage* derefSS(MemRef r) {
    return (StringStorage*)MemPoolManager::getPtr(r);
}

static uint32_t string_hash(const char* data, int len) {
    // same FNV-1a as StringStorage.cpp so hashes match
    const uint32_t FNV_PRIME = 0x01000193u, FNV_OFFSET = 0x811c9dc5u;
    uint32_t h = FNV_OFFSET;
    for (int i = 0; i < len; ++i) { h ^= (unsigned char)data[i]; h *= FNV_PRIME; }
    return h ? h : 1;
}

static int utf8_char_count(const char* s, int n) {
    int c = 0; for (int i = 0; i < n; ++i) if ((s[i] & 0xC0) != 0x80) ++c; return c;
}

// StringStorage allocated inside the MemPool with the same number as our StringPool
static MemRef allocStringStorage(uint8_t poolNum, const char* src, int lenB, uint32_t hash) {
    MemRef r = MemPoolManager::alloc(sizeof(StringStorage) + lenB + 1, poolNum);
    if (r.isNull()) return r;
    StringStorage* ss = (StringStorage*)MemPoolManager::getPtr(r);
    ss->lenB = lenB;
    ss->lenC = utf8_char_count(src, lenB);
    ss->hash = hash;
    if (lenB) memcpy(ss->data, src, lenB);
    ss->data[lenB] = '\0';
    return r;
}

static void confirmCapacity(Pool& p) {
	size_t expected = sizeof(MemRef) * p.capacity;
	if (expected > 0) {
		void* ptr = MemPoolManager::getPtr(p.stringsRef);
		size_t actual = malloc_size(ptr);
		if (actual < expected) {
			// Sanity check failed - this indicates a memory management issue
			printf("confirmCapacity error: expected >= %uz, but actual %uz\n", expected, actual);
		}
	}
}

static void initPool(uint8_t poolNum) {
    Pool& p = pools[poolNum];
    if (p.initialized) return;
    p.initialized = true;
    p.capacity = 16;
    p.count = 0;
    for (int i = 0; i < 256; ++i) p.hashTable[i] = MemRef{}; // null

    // allocate array of MemRef (StringStorage refs)
    p.stringsRef = MemPoolManager::alloc(p.capacity * sizeof(MemRef), poolNum);
    MemRef* arr = stringsArray(&p);
	confirmCapacity(p);
	
    uint32_t h = string_hash("", 0);
    MemRef emptyRef = allocStringStorage(poolNum, "", 0, h);
    arr[0] = emptyRef;
    p.count = 1;

    // add to hash
    uint8_t b = (uint8_t)(h & 0xFF);
    MemRef heRef = MemPoolManager::alloc(sizeof(HashEntry), poolNum);
    HashEntry* he = derefHE(heRef);
    he->hash = h; he->index = 0; he->next = p.hashTable[b];
    p.hashTable[b] = heRef;
}

// Look for an existing interned string that exactly matches ss.
// If found, free ss, and return the found index.
// Otherwise, adopt ss into our mem pool.
// The given StringStorage must have been allocated with malloc,
// and the caller must not free it.
uint16_t internOrAdoptString(uint8_t poolNum, StringStorage *ss) {
	if (!ss) return 0;
	initPool(poolNum);
	Pool& p = pools[poolNum];
	confirmCapacity(p);
	
	int lenB = ss_lengthB(ss);
	if (lenB == 0) return 0;
	
	const char *cstr = ss_getCString(ss);
	if (ss->hash == 0) ss->hash = string_hash(cstr, lenB);
	uint32_t h = ss->hash;
	uint8_t b = (uint8_t)(h & 0xFF);
	
	// lookup via MemRef chain
	for (MemRef eRef = p.hashTable[b]; !eRef.isNull(); eRef = derefHE(eRef)->next) {
		HashEntry* e = derefHE(eRef);
		if (e->hash != h || e->index >= p.count) continue;
		StringStorage* s = derefSS(stringsArray(&p)[e->index]);
		if (s && s->lenB == lenB && memcmp(s->data, cstr, lenB) == 0) {
			// Found a match!  Free the given ss, and return the index.
			free(ss);
			return e->index;
		}
	}
	
	// No match found -- add ss to our hash table, adopting it.
	MemRef sRef(poolNum, MemPoolManager::getPool(poolNum)->adopt(ss, ss_totalSize(ss)));
	void* wtf2 = MemPoolManager::getPtr(sRef);
	if (wtf2 != (void*)ss) return 0;
	
	uint16_t indexInStringPool = storeInPool(sRef, poolNum, h);
	
	// Sanity checks:
	const StringStorage *wtf = getStorage(poolNum, indexInStringPool);
	
	return indexInStringPool;
}

// Copy and intern a C string into our string pool.
uint16_t internString(uint8_t poolNum, const char* cstr) {
    if (!cstr) return 0;
    initPool(poolNum);
    Pool& p = pools[poolNum];
	confirmCapacity(p);
	
    int lenB = (int)strlen(cstr);
    if (lenB == 0) return 0;

    uint32_t h = string_hash(cstr, lenB);
    uint8_t b = (uint8_t)(h & 0xFF);

    // lookup via MemRef chain
    for (MemRef eRef = p.hashTable[b]; !eRef.isNull(); eRef = derefHE(eRef)->next) {
        HashEntry* e = derefHE(eRef);
        if (e->hash != h || e->index >= p.count) continue;
        StringStorage* s = derefSS(stringsArray(&p)[e->index]);
        if (s && s->lenB == lenB && memcmp(s->data, cstr, lenB) == 0) {
            return e->index;
        }
    }

    // create new StringStorage in this MemPool
    MemRef sRef = allocStringStorage(poolNum, cstr, lenB, h);
	return storeInPool(sRef, poolNum, h);
}

// Helper function to store a StringStorage (already wrapped in a MemRef) in
// our hash map and mem pool, returning its index (in the StringPool).
uint16_t storeInPool(MemRef sRef, uint8_t poolNum, uint32_t hash) {
	if (sRef.isNull()) return 0;
	// grow array of MemRef if needed
	Pool& p = pools[poolNum];
	if (p.count >= p.capacity) {
		p.capacity = (uint16_t)(p.capacity * 2);
		p.stringsRef = MemPoolManager::realloc(p.stringsRef, p.capacity * sizeof(MemRef));
	}
	confirmCapacity(p);
	MemRef* arr = stringsArray(&p);
	uint16_t idx = p.count++;
	arr[idx] = sRef;

	// add hash entry
	MemRef heRef = MemPoolManager::alloc(sizeof(HashEntry), poolNum);
	HashEntry* ne = derefHE(heRef);
	uint8_t b = (uint8_t)(hash & 0xFF);
	ne->hash = hash; ne->index = idx; ne->next = p.hashTable[b];
	p.hashTable[b] = heRef;

	return idx;
}

const StringStorage* getStorage(uint8_t poolNum, uint16_t idx) {
    Pool& p = pools[poolNum];
    if (!p.initialized || idx >= p.count) return nullptr;
	MemRef* memRef = stringsArray(&p);
	if (memRef == nullptr) return nullptr;
	MemRef entry = memRef[idx];
    StringStorage* result = derefSS(entry);
	return result;
}

const char* getCString(uint8_t poolNum, uint16_t idx) {
    const StringStorage* s = getStorage(poolNum, idx);
    return s ? s->data : "";
}

void clearPool(uint8_t poolNum) {
    // Reset StringPool state first
    Pool& p = pools[poolNum];
    if (p.initialized) {
        
        // Clear the Pool struct
        p.initialized = false;
        p.capacity = 0;
        p.count = 0;
        p.stringsRef = MemRef();  // null MemRef
        
        // Clear hash table
        for (int i = 0; i < 256; ++i) {
            p.hashTable[i] = MemRef();  // null
        }
    }
    
    // Now clear the underlying MemPool
    MemPoolManager::clearPool(poolNum);
}

StringStorage* defaultStringAllocator(const char* src, int lenB, uint32_t hash) {
    // Choose a default pool (0) or make this configurable if you like.
    MemRef r = MemPoolManager::alloc(sizeof(StringStorage) + lenB + 1, /*pool*/0);
    if (!r.isNull()) return nullptr;
    StringStorage* ss = (StringStorage*)MemPoolManager::getPtr(r);
    ss->lenB = lenB;
    ss->lenC = utf8_char_count(src, lenB);
    ss->hash = hash;
    if (lenB) memcpy(ss->data, src, lenB);
    ss->data[lenB] = '\0';
    return ss;
}

void* poolAllocator(size_t size) {
    // Use pool 0 by default
    MemRef r = MemPoolManager::alloc(size, 0);
    return r.isNull() ? nullptr : MemPoolManager::getPtr(r);
}

void* poolAllocatorForPool(size_t size, uint8_t poolNum) {
    MemRef r = MemPoolManager::alloc(size, poolNum);
    return r.isNull() ? nullptr : MemPoolManager::getPtr(r);
}

} // namespace StringPool

// C-compatible wrapper for the pool allocator
extern "C" void* stringpool_allocator(size_t size) {
    return StringPool::poolAllocator(size);
}

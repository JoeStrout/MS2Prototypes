// StringPool.cpp
#include "StringPool.h"
#include <cstring>  // strlen, memcpy

namespace StringPool {

static Pool pools[256]; // or whatever max you use

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

// StringStorage allocated inside the *same* MemPool
static MemRef allocStringStorage(uint8_t poolNum, const char* src, int lenB, uint32_t hash) {
    MemRef r = MemPoolManager::alloc(sizeof(StringStorage) + lenB + 1, poolNum);
    if (r.isNull()) return r;
    auto* ss = (StringStorage*)MemPoolManager::getPtr(r);
    ss->lenB = lenB;
    ss->lenC = utf8_char_count(src, lenB);
    ss->hash = hash;
    if (lenB) memcpy(ss->data, src, lenB);
    ss->data[lenB] = '\0';
    return r;
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
    auto* arr = stringsArray(&p);

    // index 0 = empty string
    uint32_t h = string_hash("", 0);
    MemRef emptyRef = allocStringStorage(poolNum, "", 0, h);
    arr[0] = emptyRef;
    p.count = 1;

    // add to hash
    uint8_t b = (uint8_t)(h & 0xFF);
    MemRef heRef = MemPoolManager::alloc(sizeof(HashEntry), poolNum);
    auto* he = derefHE(heRef);
    he->hash = h; he->index = 0; he->next = p.hashTable[b];
    p.hashTable[b] = heRef;
}

uint16_t internString(uint8_t poolNum, const char* cstr) {
    if (!cstr) return 0;
    initPool(poolNum);
    Pool& p = pools[poolNum];

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
    if (sRef.isNull()) return 0;

    // grow array of MemRef if needed
    if (p.count >= p.capacity) {
        p.capacity = (uint16_t)(p.capacity * 2);
        p.stringsRef = MemPoolManager::realloc(p.stringsRef, p.capacity * sizeof(MemRef));
    }
    auto* arr = stringsArray(&p);
    uint16_t idx = p.count++;
    arr[idx] = sRef;

    // add hash entry
    MemRef heRef = MemPoolManager::alloc(sizeof(HashEntry), poolNum);
    auto* ne = derefHE(heRef);
    ne->hash = h; ne->index = idx; ne->next = p.hashTable[b];
    p.hashTable[b] = heRef;

    return idx;
}

const StringStorage* getStorage(uint8_t poolNum, uint16_t idx) {
    Pool& p = pools[poolNum];
    if (!p.initialized || idx >= p.count) return nullptr;
    return derefSS(stringsArray(&p)[idx]);
}

const char* getCString(uint8_t poolNum, uint16_t idx) {
    auto* s = getStorage(poolNum, idx);
    return s ? s->data : "";
}

StringStorage* defaultStringAllocator(const char* src, int lenB, uint32_t hash) {
    // Choose a default pool (0) or make this configurable if you like.
    MemRef r = MemPoolManager::alloc(sizeof(StringStorage) + lenB + 1, /*pool*/0);
    if (!r.isNull()) return nullptr;
    auto* ss = (StringStorage*)MemPoolManager::getPtr(r);
    ss->lenB = lenB;
    ss->lenC = utf8_char_count(src, lenB);
    ss->hash = hash;
    if (lenB) memcpy(ss->data, src, lenB);
    ss->data[lenB] = '\0';
    return ss;
}

} // namespace StringPool

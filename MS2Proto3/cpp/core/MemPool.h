#pragma once
#include <cstdint>
#include <cstdlib>

// MemPool reference - trivially copyable handle
struct __attribute__((packed)) MemRef {
    uint8_t poolNum;    // Pool number (0-255)
    uint32_t index;     // Index within the pool (32-bit for larger pools)
    
    MemRef() : poolNum(0), index(0) {}
    MemRef(uint8_t pool, uint32_t idx) : poolNum(pool), index(idx) {}
    
    bool isNull() const { return index == 0; }
    
    // Comparison operators for containers
    // ToDo: replace this with `= default`?
    bool operator==(const MemRef& other) const {
        return poolNum == other.poolNum && index == other.index;
    }
    bool operator!=(const MemRef& other) const {
        return !(*this == other);
    }
};

// MemPool - manages memory blocks within a single pool
class MemPool {
private:
    struct Block {
        void* ptr;
        size_t size;
        bool inUse;
    };
    
    static const uint32_t MAX_BLOCKS = 65536;  // 64K blocks per pool
    Block* blocks;
    uint32_t blockCount;
    uint32_t capacity;
    
    uint32_t allocateBlockSlot();
    
public:
    MemPool();
    ~MemPool();
    
    // Allocate a block and return its index
    uint32_t alloc(size_t size);
    
    // Reallocate a block (similar to realloc)
    uint32_t realloc(uint32_t index, size_t newSize);
    
    // Free a specific block
    void free(uint32_t index);
    
    // Get pointer from index (returns nullptr if invalid)
    void* getPtr(uint32_t index) const;
    
    // Get size of block
    size_t getSize(uint32_t index) const;
    
    // Clear all allocations (bulk free)
    void clear();
    
    // Statistics
    uint32_t getBlockCount() const { return blockCount; }
    size_t getTotalMemory() const;
};

// Global MemPool manager
class MemPoolManager {
private:
    static MemPool* pools[256];
    
public:
    // Create a new pool (or return existing one)
    static MemPool* getPool(uint8_t poolNum);
    
    // Destroy a pool and all its allocations
    static void destroyPool(uint8_t poolNum);
    
    // Global allocation functions
    static MemRef alloc(size_t size, uint8_t poolNum = 0);
    static MemRef realloc(MemRef ref, size_t newSize);
    static void free(MemRef ref);
    static void* getPtr(MemRef ref);
    static size_t getSize(MemRef ref);
    
    // Utility functions
    static void clearPool(uint8_t poolNum);
    static void destroyAllPools();
};
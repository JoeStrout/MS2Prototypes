#include "MemPool.h"
#include <cstring>

// MemPool implementation
MemPool::MemPool() : blocks(nullptr), blockCount(1), capacity(0) {
    // Start with initial capacity
    capacity = 256;
    blocks = (Block*)malloc(capacity * sizeof(Block));
    if (blocks) {
        memset(blocks, 0, capacity * sizeof(Block));
    }
    // And note that blockCount is initialized to 1, because
    // slot 0 is reserved to represent nullptr.
}

MemPool::~MemPool() {
    clear();
    ::free(blocks);
}

uint32_t MemPool::allocateBlockSlot() {
    // Find a free slot
    for (uint32_t i = 1; i < blockCount; i++) {  // Start at 1 (0 is reserved for null)
        if (!blocks[i].inUse) {
            return i;
        }
    }
    
    // Need to grow
    if (blockCount >= capacity) {
        uint32_t newCapacity = capacity * 2;
        if (newCapacity > MAX_BLOCKS) newCapacity = MAX_BLOCKS;
        if (newCapacity <= capacity) return 0; // Out of slots
        
        Block* newBlocks = (Block*)::realloc(blocks, newCapacity * sizeof(Block));
        if (!newBlocks) return 0;
        
        // Initialize new slots
        memset(newBlocks + capacity, 0, (newCapacity - capacity) * sizeof(Block));
        
        blocks = newBlocks;
        capacity = newCapacity;
    }
    
    return blockCount++;
}

uint32_t MemPool::alloc(size_t size) {
    if (size == 0) return 0;
    
    uint32_t index = allocateBlockSlot();
    if (index == 0) return 0;
    
    void* ptr = malloc(size);
    if (!ptr) return 0;
    
    blocks[index].ptr = ptr;
    blocks[index].size = size;
    blocks[index].inUse = true;
    
    return index;
}

uint32_t MemPool::realloc(uint32_t index, size_t newSize) {
    if (index == 0 || index >= blockCount || !blocks[index].inUse) {
        // Invalid index - allocate new block
        return alloc(newSize);
    }
    
    if (newSize == 0) {
        free(index);
        return 0;
    }
    
    void* newPtr = ::realloc(blocks[index].ptr, newSize);
    if (!newPtr) return 0;  // Realloc failed
    
    blocks[index].ptr = newPtr;
    blocks[index].size = newSize;
    
    return index;
}

void MemPool::free(uint32_t index) {
    if (index == 0 || index >= blockCount || !blocks[index].inUse) {
        return;  // Invalid or already freed
    }
    
    ::free(blocks[index].ptr);
    blocks[index].ptr = nullptr;
    blocks[index].size = 0;
    blocks[index].inUse = false;
}

void* MemPool::getPtr(uint32_t index) const {
    if (index == 0 || index >= blockCount || !blocks[index].inUse) {
        return nullptr;
    }
    return blocks[index].ptr;
}

size_t MemPool::getSize(uint32_t index) const {
    if (index == 0 || index >= blockCount || !blocks[index].inUse) {
        return 0;
    }
    return blocks[index].size;
}

void MemPool::clear() {
    for (uint32_t i = 1; i < blockCount; i++) {
        if (blocks[i].inUse) {
            ::free(blocks[i].ptr);
            blocks[i].ptr = nullptr;
            blocks[i].size = 0;
            blocks[i].inUse = false;
        }
    }
    blockCount = 1;  // Keep slot 0 as null
}

size_t MemPool::getTotalMemory() const {
    size_t total = 0;
    for (uint32_t i = 1; i < blockCount; i++) {
        if (blocks[i].inUse) {
            total += blocks[i].size;
        }
    }
    return total;
}

// MemPoolManager static members
MemPool* MemPoolManager::pools[256] = {nullptr};

MemPool* MemPoolManager::getPool(uint8_t poolNum) {
    if (!pools[poolNum]) {
        pools[poolNum] = new MemPool();
    }
    return pools[poolNum];
}

void MemPoolManager::destroyPool(uint8_t poolNum) {
    if (pools[poolNum]) {
        delete pools[poolNum];
        pools[poolNum] = nullptr;
    }
}

MemRef MemPoolManager::alloc(size_t size, uint8_t poolNum) {
    MemPool* pool = getPool(poolNum);
    if (!pool) return MemRef();
    
    uint32_t index = pool->alloc(size);
    return MemRef(poolNum, index);
}

MemRef MemPoolManager::realloc(MemRef ref, size_t newSize) {
    MemPool* pool = getPool(ref.poolNum);
    if (!pool) return MemRef();
    
    uint32_t newIndex = pool->realloc(ref.index, newSize);
    return MemRef(ref.poolNum, newIndex);
}

void MemPoolManager::free(MemRef ref) {
    if (pools[ref.poolNum]) {
        pools[ref.poolNum]->free(ref.index);
    }
}

void* MemPoolManager::getPtr(MemRef ref) {
    if (!pools[ref.poolNum]) return nullptr;
    return pools[ref.poolNum]->getPtr(ref.index);
}

size_t MemPoolManager::getSize(MemRef ref) {
    if (!pools[ref.poolNum]) return 0;
    return pools[ref.poolNum]->getSize(ref.index);
}

void MemPoolManager::clearPool(uint8_t poolNum) {
    if (pools[poolNum]) {
        pools[poolNum]->clear();
    }
}

void MemPoolManager::destroyAllPools() {
    for (int i = 0; i < 256; i++) {
        destroyPool(i);
    }
}
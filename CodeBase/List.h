#pragma once
#include "MemPool.h"
#include <type_traits>
#include <new>
#include <cstring>

// Forward declaration
template<typename T> class List;

// ListStorage - the actual storage for list data
template<typename T>
struct ListStorage {
    int count;
    int capacity;
    // Data follows immediately after this struct
    // T data[capacity] would be here, but we can't use flexible array member in C++
    
    // Helper to get the data array
    T* getData() {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + sizeof(ListStorage<T>));
    }
    
    const T* getData() const {
        return reinterpret_cast<const T*>(reinterpret_cast<const char*>(this) + sizeof(ListStorage<T>));
    }
    
    // Calculate total size needed for ListStorage + capacity T items
    static size_t calculateSize(int capacity) {
        return sizeof(ListStorage<T>) + capacity * sizeof(T);
    }
    
    // Initialize a new ListStorage
    static void initialize(ListStorage<T>* storage, int initialCapacity) {
        storage->count = 0;
        storage->capacity = initialCapacity;
        // Don't need to initialize T items yet - they'll be constructed as needed
    }
};

// List - trivially copyable list using MemPool
template<typename T>
class List {
private:
    MemRef storage;  // Reference to ListStorage<T>
    
    ListStorage<T>* getStorage() const {
        return static_cast<ListStorage<T>*>(MemPoolManager::getPtr(storage));
    }
    
    void ensureCapacity(int minCapacity) {
        ListStorage<T>* s = getStorage();
        if (!s) {
            // No storage yet - create it
            createStorage(minCapacity > 4 ? minCapacity : 4);
            return;
        }
        
        if (minCapacity <= s->capacity) return;
        
        int newCapacity = s->capacity > 0 ? s->capacity * 2 : 4;
        if (newCapacity < minCapacity) newCapacity = minCapacity;
        
        // Allocate new storage
        MemRef newRef = MemPoolManager::realloc(storage, ListStorage<T>::calculateSize(newCapacity));
        if (newRef.isNull()) return; // Allocation failed
        
        storage = newRef;
        ListStorage<T>* newStorage = getStorage();
        newStorage->capacity = newCapacity;
        
        // Note: realloc preserves existing data, so count and existing T items are fine
    }
    
    void createStorage(int initialCapacity) {
        storage = MemPoolManager::alloc(ListStorage<T>::calculateSize(initialCapacity), storage.poolNum);
        if (!storage.isNull()) {
            ListStorage<T>::initialize(getStorage(), initialCapacity);
        }
    }
    
    // Helper methods for efficient copying (same as before)
    void copyElements(T* dest, const T* src, int elementCount) {
        copyElementsImpl(dest, src, elementCount, typename std::is_trivially_copyable<T>::type{});
    }
    
    void copyElementsImpl(T* dest, const T* src, int elementCount, std::true_type) {
        // Type is trivially copyable - use fast memcpy
        memcpy(dest, src, elementCount * sizeof(T));
    }
    
    void copyElementsImpl(T* dest, const T* src, int elementCount, std::false_type) {
        // Type needs proper copy construction
        for (int i = 0; i < elementCount; i++) {
            new(&dest[i]) T(src[i]);
        }
    }
    
public:
    // Constructors - all trivial!
    List(uint8_t poolNum = 0) : storage(poolNum, 0) {}
    
    // Copy constructor and assignment (trivially copyable!)
    List(const List<T>& other) = default;
    List<T>& operator=(const List<T>& other) = default;
    
    // Destructor - trivial! Memory is managed by pool lifecycle
    ~List() = default;
    
    // Constructor that adopts an array (like before)
    List(T* adoptedArray, int arrayCount, uint8_t poolNum = 0) : storage(poolNum, 0) {
        storage = MemPoolManager::alloc(ListStorage<T>::calculateSize(arrayCount), poolNum);
        if (!storage.isNull()) {
            ListStorage<T>* s = getStorage();
            ListStorage<T>::initialize(s, arrayCount);
            s->count = arrayCount;
            
            // Copy the adopted data
            copyElements(s->getData(), adoptedArray, arrayCount);
            
            // Free the original array
            free(adoptedArray);
        }
    }
    
    // Properties
    int Count() const { 
        ListStorage<T>* s = getStorage();
        return s ? s->count : 0;
    }
    
    int Capacity() const { 
        ListStorage<T>* s = getStorage();
        return s ? s->capacity : 0;
    }
    
    bool Empty() const { return Count() == 0; }
    
    // Element access
    T& operator[](int index) {
        ListStorage<T>* s = getStorage();
        return s->getData()[index];
    }
    
    const T& operator[](int index) const {
        ListStorage<T>* s = getStorage();
        return s->getData()[index];
    }
    
    T& At(int index) {
        ListStorage<T>* s = getStorage();
        if (!s || index < 0 || index >= s->count) {
            static T defaultValue = T();
            return defaultValue;
        }
        return s->getData()[index];
    }
    
    const T& At(int index) const {
        ListStorage<T>* s = getStorage();
        if (!s || index < 0 || index >= s->count) {
            static T defaultValue = T();
            return defaultValue;
        }
        return s->getData()[index];
    }
    
    // Adding elements
    void Add(const T& item) {
        ensureCapacity(Count() + 1);
        ListStorage<T>* s = getStorage();
        if (s) {
            new(&s->getData()[s->count]) T(item);
            s->count++;
        }
    }
    
    void AddRange(const List<T>& collection) {
        if (collection.Empty()) return;
        
        ensureCapacity(Count() + collection.Count());
        ListStorage<T>* s = getStorage();
        ListStorage<T>* other = collection.getStorage();
        
        if (s && other) {
            copyElements(s->getData() + s->count, other->getData(), other->count);
            s->count += other->count;
        }
    }
    
    // Clear - doesn't free memory, just resets count
    void Clear() {
        ListStorage<T>* s = getStorage();
        if (s) {
            // Destroy existing elements
            for (int i = 0; i < s->count; i++) {
                s->getData()[i].~T();
            }
            s->count = 0;
        }
    }
    
    // Iterator support
    T* begin() { 
        ListStorage<T>* s = getStorage();
        return s ? s->getData() : nullptr;
    }
    
    const T* begin() const { 
        ListStorage<T>* s = getStorage();
        return s ? s->getData() : nullptr;
    }
    
    T* end() { 
        ListStorage<T>* s = getStorage();
        return s ? s->getData() + s->count : nullptr;
    }
    
    const T* end() const { 
        ListStorage<T>* s = getStorage();
        return s ? s->getData() + s->count : nullptr;
    }
    
    // Pool management
    uint8_t getPoolNum() const { return storage.poolNum; }
    MemRef getMemRef() const { return storage; }
    
    bool isValid() const { return !storage.isNull() && getStorage() != nullptr; }
};
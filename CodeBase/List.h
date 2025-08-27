#pragma once
#include <cstdlib>
#include <cstring>
#include <new>
#include <type_traits>
#include "string.h"

template<typename T>
class List {
private:
    T* data;
    int count;
    int capacity;
    
    void ensureCapacity(int minCapacity) {
        if (minCapacity <= capacity) return;
        
        int newCapacity = capacity > 0 ? capacity * 2 : 4;
        if (newCapacity < minCapacity) newCapacity = minCapacity;
        
        T* newData = (T*)malloc(newCapacity * sizeof(T));
        if (!newData) return; // Handle allocation failure gracefully
        
        // Copy existing elements efficiently
        copyElements(newData, data, count);
        
        // Destroy old elements (only needed for non-trivially copyable types)
        for (int i = 0; i < count; i++) {
            data[i].~T();
        }
        
        free(data);
        data = newData;
        capacity = newCapacity;
    }
    
    // Helper methods for efficient copying
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
    // Constructors
    List() : data(nullptr), count(0), capacity(0) {}
    
    explicit List(int initialCapacity) : data(nullptr), count(0), capacity(0) {
        if (initialCapacity > 0) {
            ensureCapacity(initialCapacity);
        }
    }
    
    // Constructor that adopts a T* array (takes ownership)
    // Perfect for use with string.Split() result
    List(T* adoptedArray, int arrayCount) : data(adoptedArray), count(arrayCount), capacity(arrayCount) {
        // Takes ownership of the provided array - caller should not free it
    }
    
    // Copy constructor
    List(const List<T>& other) : data(nullptr), count(0), capacity(0) {
        if (other.count > 0) {
            ensureCapacity(other.count);
            copyElements(data, other.data, other.count);
            count = other.count;
        }
    }
    
    // Assignment operator
    List<T>& operator=(const List<T>& other) {
        if (this != &other) {
            Clear();
            if (other.count > 0) {
                ensureCapacity(other.count);
                copyElements(data, other.data, other.count);
                count = other.count;
            }
        }
        return *this;
    }
    
    // Destructor
    ~List() {
        Clear();
        free(data);
    }
    
    // C# List<T> API - Properties
    int Count() const { return count; }
    int Capacity() const { return capacity; }
    
    void SetCapacity(int newCapacity) {
        if (newCapacity < count) return; // Can't shrink below current count
        
        if (newCapacity == capacity) return;
        
        if (newCapacity == 0) {
            Clear();
            free(data);
            data = nullptr;
            capacity = 0;
            return;
        }
        
        T* newData = (T*)malloc(newCapacity * sizeof(T));
        if (!newData) return;
        
        for (int i = 0; i < count; i++) {
            new(&newData[i]) T(data[i]);
            data[i].~T();
        }
        
        free(data);
        data = newData;
        capacity = newCapacity;
    }
    
    // C# List<T> API - Element access
    T& operator[](int index) {
        return data[index]; // No bounds checking for performance (like C# in release mode)
    }
    
    const T& operator[](int index) const {
        return data[index];
    }
    
    T& At(int index) {
        if (index < 0 || index >= count) {
            static T defaultValue = T();
            return defaultValue; // Return reference to default value on bounds error
        }
        return data[index];
    }
    
    const T& At(int index) const {
        if (index < 0 || index >= count) {
            static T defaultValue = T();
            return defaultValue;
        }
        return data[index];
    }
    
    // C# List<T> API - Adding elements
    void Add(const T& item) {
        ensureCapacity(count + 1);
        new(&data[count]) T(item);
        count++;
    }
    
    void AddRange(const List<T>& collection) {
        if (collection.count == 0) return;
        
        ensureCapacity(count + collection.count);
        copyElements(data + count, collection.data, collection.count);
        count += collection.count;
    }
    
    void Insert(int index, const T& item) {
        if (index < 0 || index > count) return; // Bounds check
        
        ensureCapacity(count + 1);
        
        // Shift elements to the right
        for (int i = count; i > index; i--) {
            new(&data[i]) T(data[i - 1]);
            data[i - 1].~T();
        }
        
        new(&data[index]) T(item);
        count++;
    }
    
    void InsertRange(int index, const List<T>& collection) {
        if (index < 0 || index > count || collection.count == 0) return;
        
        ensureCapacity(count + collection.count);
        
        // Shift existing elements to the right
        for (int i = count - 1; i >= index; i--) {
            new(&data[i + collection.count]) T(data[i]);
            data[i].~T();
        }
        
        // Insert new elements
        copyElements(data + index, collection.data, collection.count);
        
        count += collection.count;
    }
    
    // C# List<T> API - Removing elements
    bool Remove(const T& item) {
        int index = IndexOf(item);
        if (index >= 0) {
            RemoveAt(index);
            return true;
        }
        return false;
    }
    
    void RemoveAt(int index) {
        if (index < 0 || index >= count) return;
        
        data[index].~T();
        
        // Shift elements to the left
        for (int i = index; i < count - 1; i++) {
            new(&data[i]) T(data[i + 1]);
            data[i + 1].~T();
        }
        
        count--;
    }
    
    void RemoveRange(int index, int removeCount) {
        if (index < 0 || index >= count || removeCount <= 0) return;
        
        int actualRemoveCount = removeCount;
        if (index + actualRemoveCount > count) {
            actualRemoveCount = count - index;
        }
        
        // Destroy elements being removed
        for (int i = index; i < index + actualRemoveCount; i++) {
            data[i].~T();
        }
        
        // Shift remaining elements to the left
        for (int i = index; i < count - actualRemoveCount; i++) {
            new(&data[i]) T(data[i + actualRemoveCount]);
            data[i + actualRemoveCount].~T();
        }
        
        count -= actualRemoveCount;
    }
    
    int RemoveAll(bool (*predicate)(const T&)) {
        int writeIndex = 0;
        int removedCount = 0;
        
        for (int readIndex = 0; readIndex < count; readIndex++) {
            if (predicate(data[readIndex])) {
                data[readIndex].~T();
                removedCount++;
            } else {
                if (writeIndex != readIndex) {
                    new(&data[writeIndex]) T(data[readIndex]);
                    data[readIndex].~T();
                }
                writeIndex++;
            }
        }
        
        count = writeIndex;
        return removedCount;
    }
    
    void Clear() {
        for (int i = 0; i < count; i++) {
            data[i].~T();
        }
        count = 0;
    }
    
    // C# List<T> API - Searching
    int IndexOf(const T& item) const {
        for (int i = 0; i < count; i++) {
            if (data[i] == item) {
                return i;
            }
        }
        return -1;
    }
    
    int IndexOf(const T& item, int startIndex) const {
        if (startIndex < 0 || startIndex >= count) return -1;
        
        for (int i = startIndex; i < count; i++) {
            if (data[i] == item) {
                return i;
            }
        }
        return -1;
    }
    
    int IndexOf(const T& item, int startIndex, int searchCount) const {
        if (startIndex < 0 || startIndex >= count || searchCount <= 0) return -1;
        
        int endIndex = startIndex + searchCount;
        if (endIndex > count) endIndex = count;
        
        for (int i = startIndex; i < endIndex; i++) {
            if (data[i] == item) {
                return i;
            }
        }
        return -1;
    }
    
    int LastIndexOf(const T& item) const {
        for (int i = count - 1; i >= 0; i--) {
            if (data[i] == item) {
                return i;
            }
        }
        return -1;
    }
    
    int LastIndexOf(const T& item, int startIndex) const {
        if (startIndex < 0 || startIndex >= count) return -1;
        
        for (int i = startIndex; i >= 0; i--) {
            if (data[i] == item) {
                return i;
            }
        }
        return -1;
    }
    
    bool Contains(const T& item) const {
        return IndexOf(item) >= 0;
    }
    
    // C# List<T> API - Utility methods
    void Reverse() {
        if (count <= 1) return;
        
        for (int i = 0; i < count / 2; i++) {
            T temp = data[i];
            data[i] = data[count - 1 - i];
            data[count - 1 - i] = temp;
        }
    }
    
    void Reverse(int index, int length) {
        if (index < 0 || length <= 0 || index + length > count) return;
        
        int start = index;
        int end = index + length - 1;
        
        while (start < end) {
            T temp = data[start];
            data[start] = data[end];
            data[end] = temp;
            start++;
            end--;
        }
    }
    
    void Sort() {
        if (count <= 1) return;
        quickSort(0, count - 1);
    }
    
    void Sort(int (*comparer)(const T&, const T&)) {
        if (count <= 1) return;
        quickSortWithComparer(0, count - 1, comparer);
    }
    
    List<T> GetRange(int index, int length) const {
        List<T> result;
        if (index < 0 || length <= 0 || index + length > count) return result;
        
        result.ensureCapacity(length);
        for (int i = 0; i < length; i++) {
            result.Add(data[index + i]);
        }
        
        return result;
    }
    
    // C# List<T> API - Conversion
    T* ToArray(int* arraySize) const {
        if (count == 0) {
            *arraySize = 0;
            return nullptr;
        }
        
        T* array = (T*)malloc(count * sizeof(T));
        if (!array) {
            *arraySize = 0;
            return nullptr;
        }
        
        // Use const_cast to call non-const method (safe for copying)
        const_cast<List<T>*>(this)->copyElements(array, data, count);
        
        *arraySize = count;
        return array;
    }
    
    void TrimExcess() {
        if (count < capacity * 0.9) { // Only trim if significantly over-allocated
            SetCapacity(count);
        }
    }
    
    // Additional useful methods
    bool Empty() const { return count == 0; }
    
    T& First() { return data[0]; }
    const T& First() const { return data[0]; }
    
    T& Last() { return data[count - 1]; }
    const T& Last() const { return data[count - 1]; }
    
    // Iterator support (basic)
    T* begin() { return data; }
    const T* begin() const { return data; }
    
    T* end() { return data + count; }
    const T* end() const { return data + count; }

private:
    // QuickSort implementation for Sort()
    void quickSort(int low, int high) {
        if (low < high) {
            int pivot = partition(low, high);
            quickSort(low, pivot - 1);
            quickSort(pivot + 1, high);
        }
    }
    
    int partition(int low, int high) {
        T pivot = data[high];
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            if (data[j] < pivot) {
                i++;
                T temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
        
        T temp = data[i + 1];
        data[i + 1] = data[high];
        data[high] = temp;
        
        return i + 1;
    }
    
    void quickSortWithComparer(int low, int high, int (*comparer)(const T&, const T&)) {
        if (low < high) {
            int pivot = partitionWithComparer(low, high, comparer);
            quickSortWithComparer(low, pivot - 1, comparer);
            quickSortWithComparer(pivot + 1, high, comparer);
        }
    }
    
    int partitionWithComparer(int low, int high, int (*comparer)(const T&, const T&)) {
        T pivot = data[high];
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            if (comparer(data[j], pivot) < 0) {
                i++;
                T temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
        
        T temp = data[i + 1];
        data[i + 1] = data[high];
        data[high] = temp;
        
        return i + 1;
    }
};

// Helper function for string::Join with List<string>
// This needs to be after the List class definition
namespace ListStringJoin {
    static string Join(const string& separator, const List<string>& values, uint8_t pool = 0) {
        // Simply delegate to string::Join using public methods
        return string::Join(separator, values.begin(), values.Count(), pool);
    }
}
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdlib>  // For malloc/free
#include "StringStorage.h"
#include "StringPool.h"
#include "CS_List.h"

// Forward declaration to avoid circular dependency
template<typename T> class List;

// Lightweight string class - thin wrapper around StringPool
class string {
private:
    uint8_t poolNum;    // Intern pool number (0-255)
    uint16_t index;     // Index within the pool
    
    // Helper to create string from StringStorage (takes ownership)
    static string fromStorage(StringStorage* storage, uint8_t pool = 0) {
        if (!storage) return string();
        uint16_t idx = StringPool::internString(pool, storage->getCString());
        StringStorage::destroy(storage);  // Clean up temporary storage
        return string(pool, idx);
    }
    
    // Private constructor from pool and index
    string(uint8_t pool, uint16_t idx) : poolNum(pool), index(idx) {}
    
public:
    
    // Constructors
    string() : poolNum(0), index(0) {}
    
    string(const char* cstr, uint8_t pool = 0) : poolNum(pool) {
        if (!cstr) {
            index = 0;
            return;
        }
        index = StringPool::internString(pool, cstr);
    }
    
    // Copy constructor and assignment (defaulted for trivial copyability)
    string(const string& other) = default;
    string& operator=(const string& other) = default;
    
    // Assignment from C string
    string& operator=(const char* cstr) {
        if (!cstr) {
            poolNum = 0;
            index = 0;
            return *this;
        }
        // Use current pool and static allocator
        index = StringPool::internString(poolNum, cstr);
        return *this;
    }
    
    // Basic operations (inline wrappers)
    int lengthB() const { 
        const StringStorage* s = getStorage(); 
        return s ? s->lengthB() : 0; 
    }
    
    int lengthC() const { 
        const StringStorage* s = getStorage(); 
        return s ? s->lengthC() : 0; 
    }
    
    const char* c_str() const { 
        return StringPool::getCString(poolNum, index); 
    }
    
    // String concatenation
    string operator+(const string& other) const {
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 || !s2) return string();
        
        StringStorage* result = s1->concat(s2);
        return fromStorage(result, poolNum);
    }
    
    // Comparison
    bool operator==(const string& other) const {
        if (poolNum == other.poolNum && index == other.index) return true;
        
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 || !s2) return false;
        
        return s1->equals(s2);
    }
    
    bool operator!=(const string& other) const { return !(*this == other); }
    
    bool operator<(const string& other) const {
        return Compare(other) < 0;
    }
    
    bool operator<=(const string& other) const {
        return Compare(other) <= 0;
    }
    
    bool operator>(const string& other) const {
        return Compare(other) > 0;
    }
    
    bool operator>=(const string& other) const {
        return Compare(other) >= 0;
    }
    
    // C# String API - Properties
    int Length() const { return lengthC(); }
    bool Empty() const { return Length() == 0; }
    
    // C# String API - Character access
    char operator[](int index) const {
        const StringStorage* s = getStorage();
        return s ? s->charAt(index) : '\0';
    }
    
    // C# String API - Search methods
    int IndexOf(const string& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? s->indexOf(needle) : -1;
    }
    
    int IndexOf(const string& value, int startIndex) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? s->indexOf(needle, startIndex) : -1;
    }
    
    int IndexOf(char value) const {
        const StringStorage* s = getStorage();
        return s ? s->indexOf(value) : -1;
    }
    
    int IndexOf(char value, int startIndex) const {
        const StringStorage* s = getStorage();
        return s ? s->indexOf(value, startIndex) : -1;
    }
    
    int LastIndexOf(const string& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? s->lastIndexOf(needle) : -1;
    }
    
    int LastIndexOf(char value) const {
        const StringStorage* s = getStorage();
        return s ? s->lastIndexOf(value) : -1;
    }
    
    bool Contains(const string& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? s->contains(needle) : false;
    }
    
    bool StartsWith(const string& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* prefix = value.getStorage();
        return (s && prefix) ? s->startsWith(prefix) : false;
    }
    
    bool EndsWith(const string& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* suffix = value.getStorage();
        return (s && suffix) ? s->endsWith(suffix) : false;
    }
    
    // C# String API - Manipulation methods
    string Substring(int startIndex) const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->substring(startIndex);
        return fromStorage(result, poolNum);
    }
    
    string Substring(int startIndex, int length) const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->substring(startIndex, length);
        return fromStorage(result, poolNum);
    }
    
    string Insert(int startIndex, const string& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* v = value.getStorage();
        if (!s || !v) return string();
        
        StringStorage* result = s->insert(startIndex, v);
        return fromStorage(result, poolNum);
    }
    
    string Remove(int startIndex) const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->remove(startIndex);
        return fromStorage(result, poolNum);
    }
    
    string Remove(int startIndex, int count) const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->remove(startIndex, count);
        return fromStorage(result, poolNum);
    }
    
    string Replace(const string& oldValue, const string& newValue) const {
        const StringStorage* s = getStorage();
        const StringStorage* oldVal = oldValue.getStorage();
        const StringStorage* newVal = newValue.getStorage();
        if (!s || !oldVal || !newVal) return string();
        
        StringStorage* result = s->replace(oldVal, newVal);
        return fromStorage(result, poolNum);
    }
    
    string Replace(char oldChar, char newChar) const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->replace(oldChar, newChar);
        return fromStorage(result, poolNum);
    }
    
    // C# String API - Case conversion
    string ToLower() const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->toLower();
        return fromStorage(result, poolNum);
    }
    
    string ToUpper() const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->toUpper();
        return fromStorage(result, poolNum);
    }
    
    // C# String API - Trimming
    string Trim() const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->trim();
        return fromStorage(result, poolNum);
    }
    
    string TrimStart() const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->trimStart();
        return fromStorage(result, poolNum);
    }
    
    string TrimEnd() const {
        const StringStorage* s = getStorage();
        if (!s) return string();
        
        StringStorage* result = s->trimEnd();
        return fromStorage(result, poolNum);
    }
    
    // C# String API - Splitting (caller must free returned array and its contents)
    string* Split(char separator, int* count) const {
        const StringStorage* s = getStorage();
        if (!s) {
            *count = 0;
            return nullptr;
        }
        
        StringStorage** parts = s->split(separator, count);
        if (!parts) return nullptr;
        
        string* result = (string*)malloc(*count * sizeof(string));
        for (int i = 0; i < *count; i++) {
            result[i] = fromStorage(parts[i], poolNum);
        }
        free(parts);  // Free the array, but not the contents (transferred to result)
        
        return result;
    }
    
    string* Split(const string& separator, int* count) const {
        const StringStorage* s = getStorage();
        const StringStorage* sep = separator.getStorage();
        if (!s || !sep) {
            *count = 0;
            return nullptr;
        }
        
        StringStorage** parts = s->split(sep, count);
        if (!parts) return nullptr;
        
        string* result = (string*)malloc(*count * sizeof(string));
        for (int i = 0; i < *count; i++) {
            result[i] = fromStorage(parts[i], poolNum);
        }
        free(parts);
        
        return result;
    }
    
    // List-based Split methods - return List<string> instead of string*
    List<string> SplitToList(char separator, uint8_t listPool = 0) const {
        const StringStorage* s = getStorage();
        List<string> result(listPool);
        if (!s) return result;
        
        int count = 0;
        StringStorage** parts = s->split(separator, &count);
        if (!parts) return result;
        
        // Add all parts to the list
        for (int i = 0; i < count; i++) {
            string part = fromStorage(parts[i], poolNum);
            result.Add(part);
        }
        
        free(parts);  // Free the array, but not the contents (transferred to List)
        return result;
    }
    
    List<string> SplitToList(const string& separator, uint8_t listPool = 0) const {
        const StringStorage* s = getStorage();
        const StringStorage* sep = separator.getStorage();
        List<string> result(listPool);
        if (!s || !sep) return result;
        
        int count = 0;
        StringStorage** parts = s->split(sep, &count);
        if (!parts) return result;
        
        // Add all parts to the list
        for (int i = 0; i < count; i++) {
            string part = fromStorage(parts[i], poolNum);
            result.Add(part);
        }
        
        free(parts);
        return result;
    }
    
    // C# String API - Static methods (as regular methods)
    bool IsNullOrEmpty() const { return Length() == 0; }
    
    bool IsNullOrWhiteSpace() const {
        const StringStorage* s = getStorage();
        return s ? s->isNullOrWhiteSpace() : true;
    }
    
    // C# String API - Comparison (case-insensitive)
    bool Equals(const string& other) const { return *this == other; }
    
    int Compare(const string& other) const {
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 && !s2) return 0;
        if (!s1) return -1;
        if (!s2) return 1;
        return s1->compare(s2);
    }
    
    int CompareIgnoreCase(const string& other) const {
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 && !s2) return 0;
        if (!s1) return -1;
        if (!s2) return 1;
        return s1->compareIgnoreCase(s2);
    }
    
    // C# String API - Static methods (Join)
    static string Join(const string& separator, const string* values, int count, uint8_t pool = 0) {
        if (!values || count <= 0) return string();
        if (count == 1) return values[0];
        
        // Calculate total length needed
        int totalLength = 0;
        const StringStorage* sepStorage = separator.getStorage();
        int sepLength = sepStorage ? sepStorage->lengthB() : 0;
        
        for (int i = 0; i < count; i++) {
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                totalLength += valueStorage->lengthB();
            }
        }
        totalLength += sepLength * (count - 1); // separators between elements
        
        if (totalLength == 0) return string();
        
        // Build the result string
        char* result = (char*)malloc(totalLength + 1);
        if (!result) return string();
        
        int pos = 0;
        const char* sepCStr = sepStorage ? sepStorage->getCString() : "";
        
        for (int i = 0; i < count; i++) {
            if (i > 0 && sepLength > 0) {
                strcpy(result + pos, sepCStr);
                pos += sepLength;
            }
            
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                const char* valueCStr = valueStorage->getCString();
                int valueLength = valueStorage->lengthB();
                strcpy(result + pos, valueCStr);
                pos += valueLength;
            }
        }
        result[totalLength] = '\0';
        
        // Create string from result and clean up
        string joined(result, pool);
        free(result);
        return joined;
    }
    
    // Join overload that takes List<string>
    static string Join(const string& separator, const List<string>& values, uint8_t pool = 0) {
        int count = values.Count();
        if (count <= 0) return string();
        if (count == 1) return values[0];
        
        // Calculate total length needed
        int totalLength = 0;
        const StringStorage* sepStorage = separator.getStorage();
        int sepLength = sepStorage ? sepStorage->lengthB() : 0;
        
        for (int i = 0; i < count; i++) {
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                totalLength += valueStorage->lengthB();
            }
        }
        totalLength += sepLength * (count - 1); // separators between elements
        
        if (totalLength == 0) return string();
        
        // Build the result string
        char* result = (char*)malloc(totalLength + 1);
        if (!result) return string();
        
        int pos = 0;
        const char* sepCStr = sepStorage ? sepStorage->getCString() : "";
        
        for (int i = 0; i < count; i++) {
            if (i > 0 && sepLength > 0) {
                strcpy(result + pos, sepCStr);
                pos += sepLength;
            }
            
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                const char* valueCStr = valueStorage->getCString();
                int valueLength = valueStorage->lengthB();
                strcpy(result + pos, valueCStr);
                pos += valueLength;
            }
        }
        result[totalLength] = '\0';
        
        // Create string from result and clean up
        string joined(result, pool);
        free(result);
        return joined;
    }
    
    // Access to pool info (for debugging/advanced use)
    uint8_t getPoolNum() const { return poolNum; }
    uint16_t getIndex() const { return index; }
    const StringStorage* getStorage() const { 
        return StringPool::getStorage(poolNum, index); 
    }
};
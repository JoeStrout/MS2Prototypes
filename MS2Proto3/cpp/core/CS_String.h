// This header defines the String class, which is our equivalent of
// a C# String, and is used with C++ code transpiled from C#.
// It uses a MemPool for all its memory allocations.
//
// (Called ms_string.h because String.h conflicts with a standard 
// C++ library header.)

#pragma once
#include <cstdint>
#include <cstring>
#include <cstdlib>  // For malloc/free
#include "StringStorage.h"
#include "StringPool.h"
#include "CS_List.h"
#include <cstdio> // for debugging

// Forward declaration to avoid circular dependency
template<typename T> class List;

// Lightweight String class - thin wrapper around StringPool
class String {
private:
    uint8_t poolNum;    // Intern pool number (0-255)
    uint16_t index;     // Index within the StringPool
    
    // Helper to create String from StringStorage allocated by malloc; the given
	// storage is adopted into our MemPool, and should not be freed by the caller.
    static String fromMallocStorage(StringStorage* storage, uint8_t pool = 0) {
        if (!storage) return String();
		uint16_t idx = StringPool::internOrAdoptString(pool, storage); // adopts or frees storage
        return String(pool, idx);
    }
    
    // Private constructor from pool and index
    String(uint8_t pool, uint16_t idx) : poolNum(pool), index(idx) {}
    
public:
    
    // Constructors
    String() : poolNum(0), index(0) {}
    
    String(const char* cstr, uint8_t pool = 0) : poolNum(pool) {
        if (!cstr) {
            index = 0;
            return;
        }
        index = StringPool::internString(pool, cstr);
    }
    
    // Copy constructor and assignment (defaulted for trivial copyability)
    String(const String& other) = default;
    String& operator=(const String& other) = default;
    
    // Assignment from C string
    String& operator=(const char* cstr) {
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
        return ss_lengthB(s);
    }
    
    int lengthC() const { 
        const StringStorage* s = getStorage(); 
        return ss_lengthC(s);
    }
    
    const char* c_str() const { 
        return StringPool::getCString(poolNum, index); 
    }
    
    // String concatenation
    String operator+(const String& other) const {
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 || !s2) return String();

        StringStorage* result_ss = ss_concat(s1, s2, malloc);
        String result = fromMallocStorage(result_ss, poolNum);
		return result;
    }

    // String concatenation assignment
	// NOTE: This operation abandons our previous StringStorage to the MemPool
	// it is in.
    String& operator+=(const String& other) {
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 || !s2) return *this;

        StringStorage* result_ss = ss_concat(s1, s2, malloc);
        if (result_ss) {
            index = StringPool::internOrAdoptString(poolNum, result_ss);
        }
        return *this;
    }
    
    // Comparison
    bool operator==(const String& other) const {
        if (poolNum == other.poolNum && index == other.index) return true;
        
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 || !s2) return false;
        
        return ss_equals(s1, s2);
    }
    
    bool operator!=(const String& other) const { return !(*this == other); }
    
    bool operator<(const String& other) const {
        return Compare(other) < 0;
    }
    
    bool operator<=(const String& other) const {
        return Compare(other) <= 0;
    }
    
    bool operator>(const String& other) const {
        return Compare(other) > 0;
    }
    
    bool operator>=(const String& other) const {
        return Compare(other) >= 0;
    }
    
    // C# String API - Properties
    int Length() const { return lengthC(); }
    bool Empty() const { return Length() == 0; }
    
    // C# String API - Character access
    char operator[](int index) const {
        const StringStorage* s = getStorage();
        return ss_charAt(s, index);
    }
    
    // C# String API - Search methods
    int IndexOf(const String& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? ss_indexOf(s, needle) : -1;
    }
    
    int IndexOf(const String& value, int startIndex) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? ss_indexOfFrom(s, needle, startIndex) : -1;
    }
    
    int IndexOf(char value) const {
        const StringStorage* s = getStorage();
        return ss_indexOfChar(s, value);
    }
    
    int IndexOf(char value, int startIndex) const {
        const StringStorage* s = getStorage();
        return ss_indexOfCharFrom(s, value, startIndex);
    }
    
    int LastIndexOf(const String& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? ss_lastIndexOf(s, needle) : -1;
    }
    
    int LastIndexOf(char value) const {
        const StringStorage* s = getStorage();
        return ss_lastIndexOfChar(s, value);
    }
    
    bool Contains(const String& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* needle = value.getStorage();
        return (s && needle) ? ss_contains(s, needle) : false;
    }
    
    bool StartsWith(const String& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* prefix = value.getStorage();
        return (s && prefix) ? ss_startsWith(s, prefix) : false;
    }
    
    bool EndsWith(const String& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* suffix = value.getStorage();
        return (s && suffix) ? ss_endsWith(s, suffix) : false;
    }
    
    // C# String API - Manipulation methods
    String Substring(int startIndex) const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result = ss_substring(s, startIndex, malloc);
        return fromMallocStorage(result, poolNum);
    }
    
    String Substring(int startIndex, int length) const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_substringLen(s, startIndex, length, malloc);
        return fromMallocStorage(result_ss, poolNum);
    }
    
    String Left(int chars) const {
    	return Substring(0, chars);
    }
    
    String Right(int chars) const {
    	int len = Length();
    	if (chars > len) return *this;
    	return Substring(len - chars, chars);
    }
    
    String Insert(int startIndex, const String& value) const {
        const StringStorage* s = getStorage();
        const StringStorage* v = value.getStorage();
        if (!s || !v) return String();
        
        StringStorage* result_ss = ss_insert(s, startIndex, v, malloc);
		return fromMallocStorage(result_ss, poolNum);
    }
    
    String Remove(int startIndex) const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_remove(s, startIndex, malloc);
		return fromMallocStorage(result_ss, poolNum);
    }
    
    String Remove(int startIndex, int count) const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_removeLen(s, startIndex, count, malloc);
		return fromMallocStorage(result_ss, poolNum);
    }
    
    String Replace(const String& oldValue, const String& newValue) const {
        const StringStorage* s = getStorage();
        const StringStorage* oldVal = oldValue.getStorage();
        const StringStorage* newVal = newValue.getStorage();
        if (!s || !oldVal || !newVal) return String();
        
        StringStorage* result_ss = ss_replace(s, oldVal, newVal, malloc);
		return fromMallocStorage(result_ss, poolNum);
    }
    
    String Replace(char oldChar, char newChar) const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_replaceChar(s, oldChar, newChar, malloc);
		return fromMallocStorage(result_ss, poolNum);
    }
    
    // C# String API - Case conversion
    String ToLower() const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_toLower(s, malloc);
        return fromMallocStorage(result_ss, poolNum);
    }
    
    String ToUpper() const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_toUpper(s, malloc);
        return fromMallocStorage(result_ss, poolNum);
    }
    
    // C# String API - Trimming
    String Trim() const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_trim(s, malloc);
        return fromMallocStorage(result_ss, poolNum);
    }
    
    String TrimStart() const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_trimStart(s, malloc);
        return fromMallocStorage(result_ss, poolNum);
    }
    
    String TrimEnd() const {
        const StringStorage* s = getStorage();
        if (!s) return String();
        
        StringStorage* result_ss = ss_trimEnd(s, malloc);
        return fromMallocStorage(result_ss, poolNum);
    }
    
    // C# String API - Splitting (caller must free returned array and its contents)
    String* Split(char separator, int* count) const {
        const StringStorage* s = getStorage();
        if (!s) {
            *count = 0;
            return nullptr;
        }
        
        StringStorage** parts = ss_split(s, separator, count, malloc);
        if (!parts) return nullptr;
        
        String* result = (String*)malloc(*count * sizeof(String));
        for (int i = 0; i < *count; i++) {
            result[i] = fromMallocStorage(parts[i], poolNum);
        }
        free(parts);  // Free the array, but not the contents (transferred to result)
        
        return result;
    }
    
    String* Split(const String& separator, int* count) const {
        const StringStorage* s = getStorage();
        const StringStorage* sep = separator.getStorage();
        if (!s || !sep) {
            *count = 0;
            return nullptr;
        }
        
        StringStorage** parts = ss_splitStr(s, sep, count, malloc);
        if (!parts) return nullptr;
        
        String* result = (String*)malloc(*count * sizeof(String));
        for (int i = 0; i < *count; i++) {
            result[i] = fromMallocStorage(parts[i], poolNum);
        }
        free(parts);
        
        return result;
    }
    
    // List-based Split methods - return List<String> instead of String*
    List<String> SplitToList(char separator, uint8_t listPool = 0) const {
        const StringStorage* s = getStorage();
        List<String> result(listPool);
        if (!s) return result;
        
        int count = 0;
        StringStorage** parts = ss_split(s, separator, &count, malloc);
        if (!parts) return result;
        
        // Add all parts to the list
		// OFI: add some List API to let us directly set the capacity to `count`
        for (int i = 0; i < count; i++) {
            String part = fromMallocStorage(parts[i], poolNum);
            result.Add(part);
        }
        
        free(parts);  // Free the array, but not the contents (transferred to List)
        return result;
    }
    
    List<String> SplitToList(const String& separator, uint8_t listPool = 0) const {
        const StringStorage* s = getStorage();
        const StringStorage* sep = separator.getStorage();
        List<String> result(listPool);
        if (!s || !sep) return result;
        
        int count = 0;
        StringStorage** parts = ss_splitStr(s, sep, &count, malloc);
        if (!parts) return result;
        
        // Add all parts to the list
        for (int i = 0; i < count; i++) {
            String part = fromMallocStorage(parts[i], poolNum);
            result.Add(part);
        }
        
        free(parts);
        return result;
    }
    
    // C# String API - Static methods (as regular methods)
    static bool IsNullOrEmpty(const String& s) { return s.Length() == 0; }
    
    bool IsNullOrWhiteSpace() const {
        const StringStorage* s = getStorage();
        return ss_isNullOrWhiteSpace(s);
    }
    
    // C# String API - Comparison (case-insensitive)
    bool Equals(const String& other) const { return *this == other; }
    
    int Compare(const String& other) const {
        const StringStorage* s1 = getStorage();
        const StringStorage* s2 = other.getStorage();
        if (!s1 && !s2) return 0;
        if (!s1) return -1;
        if (!s2) return 1;
        return ss_compare(s1, s2);
    }
    
    // C# String API - Static methods (Join)
    static String Join(const String& separator, const String* values, int count, uint8_t pool = 0) {
        if (!values || count <= 0) return String();
        if (count == 1) return values[0];
        
        // Calculate total length needed
        int totalLength = 0;
        const StringStorage* sepStorage = separator.getStorage();
        int sepLength = sepStorage ? ss_lengthB(sepStorage) : 0;
        
        for (int i = 0; i < count; i++) {
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                totalLength += ss_lengthB(valueStorage);
            }
        }
        totalLength += sepLength * (count - 1); // separators between elements
        
        if (totalLength == 0) return String();
        
        // Build the result String
        char* result = (char*)malloc(totalLength + 1); // malloc valid here because we free it below
        if (!result) return String();
        
        int pos = 0;
        const char* sepCStr = ss_getCString(sepStorage);
        
        for (int i = 0; i < count; i++) {
            if (i > 0 && sepLength > 0) {
                strcpy(result + pos, sepCStr);  // ToDo: replace with memcpy
                pos += sepLength;
            }
            
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                const char* valueCStr = ss_getCString(valueStorage);
                int valueLength = ss_lengthB(valueStorage);
                strcpy(result + pos, valueCStr);  // ToDo: replace with memcpy
                pos += valueLength;
            }
        }
        result[totalLength] = '\0';
        
        // Create String from result and clean up
        String joined(result, pool);
        ::free(result);		// free valid here because it came from malloc, above
        return joined;
    }
    
    // Join overload that takes List<String>
    static String Join(const String& separator, const List<String>& values, uint8_t pool = 0) {
        int count = values.Count();
        if (count <= 0) return String();
        if (count == 1) return values[0];
        
        // Calculate total length needed
        int totalLength = 0;
        const StringStorage* sepStorage = separator.getStorage();
        int sepLength = sepStorage ? ss_lengthB(sepStorage) : 0;
        
        for (int i = 0; i < count; i++) {
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                totalLength += ss_lengthB(valueStorage);
            }
        }
        totalLength += sepLength * (count - 1); // separators between elements
        
        if (totalLength == 0) return String();
        
        // Build the result String
        char* result = (char*)malloc(totalLength + 1);	// malloc valid here because we free it below
        if (!result) return String();
        
        int pos = 0;
        const char* sepCStr = sepStorage ? ss_getCString(sepStorage) : "";
        
        for (int i = 0; i < count; i++) {
            if (i > 0 && sepLength > 0) {
                strcpy(result + pos, sepCStr);
                pos += sepLength;
            }
            
            const StringStorage* valueStorage = values[i].getStorage();
            if (valueStorage) {
                const char* valueCStr = ss_getCString(valueStorage);
                int valueLength = ss_lengthB(valueStorage);
                strcpy(result + pos, valueCStr);
                pos += valueLength;
            }
        }
        result[totalLength] = '\0';
        
        // Create String from result and clean up
        String joined(result, pool);
        free(result);					// free valid here because it came from malloc, above
        return joined;
    }
    
    // Access to pool info (for debugging/advanced use)
    uint8_t getPoolNum() const { return poolNum; }
    uint16_t getIndex() const { return index; }
    const StringStorage* getStorage() const {
        return StringPool::getStorage(poolNum, index);
    }
};

// Free function: C string + String concatenation
inline String operator+(const char* lhs, const String& rhs) {
    if (!lhs) return rhs;
    String temp(lhs, rhs.getPoolNum());
    return temp + rhs;
}

#pragma once
#include <cstdint>
#include "unicodeUtil.h"

// Forward declarations
struct StringStorage;
typedef StringStorage* (*StringStorageAllocator)(const char* source, int byteLen, uint32_t hash);

// Core string storage structure (heap-allocated)
// This structure and its associated functions are completely independent 
// of any string class or pool management
struct StringStorage {
    int lenB;           // Length in bytes
    int lenC;           // Length in characters (UTF-8)
    uint32_t hash;      // String hash for fast comparison
    char data[];        // Flexible array member for string data
    
    // Public static allocator - can be changed by user
    static StringStorageAllocator allocator;
    
    // Create a new StringStorage from a C string
    static StringStorage* create(const char* cstr);
    
    // Create a new StringStorage with specified length (data uninitialized)
    static StringStorage* createWithLength(int byteLen);
    
    // Destroy a StringStorage (free memory)
    static void destroy(StringStorage* storage);
    
    // Get C string pointer (data is always null-terminated)
    const char* getCString() const { return data; }
    
    // Basic properties
    int lengthB() const { return lenB; }
    int lengthC() const { return lenC; }
    bool isEmpty() const { return lenB == 0; }
    
    // Character access (byte-based indexing)
    char charAt(int byteIndex) const;
    
    // Comparison
    bool equals(const StringStorage* other) const;
    int compare(const StringStorage* other) const;
    bool equalsIgnoreCase(const StringStorage* other) const;
    int compareIgnoreCase(const StringStorage* other) const;
    
    // Search methods
    int indexOf(const StringStorage* needle) const;
    int indexOf(const StringStorage* needle, int startIndex) const;
    int indexOf(char ch) const;
    int indexOf(char ch, int startIndex) const;
    int lastIndexOf(const StringStorage* needle) const;
    int lastIndexOf(char ch) const;
    bool contains(const StringStorage* needle) const;
    bool startsWith(const StringStorage* prefix) const;
    bool endsWith(const StringStorage* suffix) const;
    
    // String manipulation (returns new StringStorage instances)
    StringStorage* substring(int startIndex) const;
    StringStorage* substring(int startIndex, int length) const;
    StringStorage* concat(const StringStorage* other) const;
    StringStorage* insert(int startIndex, const StringStorage* value) const;
    StringStorage* remove(int startIndex) const;
    StringStorage* remove(int startIndex, int count) const;
    StringStorage* replace(const StringStorage* oldValue, const StringStorage* newValue) const;
    StringStorage* replace(char oldChar, char newChar) const;
    
    // Case conversion (ASCII only)
    StringStorage* toLower() const;
    StringStorage* toUpper() const;
    
    // Trimming (Unicode-aware)
    StringStorage* trim() const;
    StringStorage* trimStart() const;
    StringStorage* trimEnd() const;
    
    // Whitespace checking (Unicode-aware)
    bool isNullOrWhiteSpace() const;
    
    // Splitting (caller must free returned array and its contents)
    StringStorage** split(char separator, int* count) const;
    StringStorage** split(const StringStorage* separator, int* count) const;
    
    // Hash computation
    uint32_t computeHash() const;
    void ensureHashComputed();
    
private:
    // Helper methods for internal use
    static int utf8CharCount(const char* str, int byteLen);
    static char asciiToLower(char c);
    static char asciiToUpper(char c);
};
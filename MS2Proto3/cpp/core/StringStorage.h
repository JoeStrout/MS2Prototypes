// Core string storage structure (heap-allocated).
// This structure and its associated functions are completely independent 
// of any string class or pool management, i.e., it can be used with any
// allocator.  It underlies heap-allocated strings for both the host String
// (C#-like) class, and for (MiniScript runtime) Value strings, even though
// those are allocated and collected in different ways.

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>  // For size_t

#ifdef __cplusplus
extern "C" {
#endif

#include "unicodeUtil.h"

typedef struct StringStorage {
    int lenB;           // Length in bytes
    int lenC;           // Length in characters (UTF-8)
    uint32_t hash;      // String hash for fast comparison
    char data[];        // Flexible array member for string data
} StringStorage;

// Allocator function type for StringStorage
// size: total number of bytes to allocate (sizeof(StringStorage) + stringLength + 1)
// Returns: allocated memory block, or NULL on failure
typedef void* (*StringStorageAllocator)(size_t size);

// Core StringStorage functions (ss_ prefix for "string storage")

// Creation and destruction
StringStorage* ss_create(const char* cstr, StringStorageAllocator allocator);
StringStorage* ss_createWithLength(int byteLen, StringStorageAllocator allocator);
void ss_destroy(StringStorage* storage);

// Basic accessors
const char* ss_getCString(const StringStorage* storage);
int ss_lengthB(const StringStorage* storage);
int ss_lengthC(const StringStorage* storage);
bool ss_isEmpty(const StringStorage* storage);

// Character access (byte-based indexing)
char ss_charAt(const StringStorage* storage, int byteIndex);

// Comparison
bool ss_equals(const StringStorage* storage, const StringStorage* other);
int ss_compare(const StringStorage* storage, const StringStorage* other);
bool ss_equalsIgnoreCase(const StringStorage* storage, const StringStorage* other);
int ss_compareIgnoreCase(const StringStorage* storage, const StringStorage* other);

// Search methods
int ss_indexOf(const StringStorage* storage, const StringStorage* needle);
int ss_indexOfFrom(const StringStorage* storage, const StringStorage* needle, int startIndex);
int ss_indexOfChar(const StringStorage* storage, char ch);
int ss_indexOfCharFrom(const StringStorage* storage, char ch, int startIndex);
int ss_lastIndexOf(const StringStorage* storage, const StringStorage* needle);
int ss_lastIndexOfChar(const StringStorage* storage, char ch);
bool ss_contains(const StringStorage* storage, const StringStorage* needle);
bool ss_startsWith(const StringStorage* storage, const StringStorage* prefix);
bool ss_endsWith(const StringStorage* storage, const StringStorage* suffix);

// String manipulation (returns new StringStorage instances)
StringStorage* ss_substring(const StringStorage* storage, int startIndex, StringStorageAllocator allocator);
StringStorage* ss_substringLen(const StringStorage* storage, int startIndex, int length, StringStorageAllocator allocator);
StringStorage* ss_concat(const StringStorage* storage, const StringStorage* other, StringStorageAllocator allocator);
StringStorage* ss_insert(const StringStorage* storage, int startIndex, const StringStorage* value, StringStorageAllocator allocator);
StringStorage* ss_remove(const StringStorage* storage, int startIndex, StringStorageAllocator allocator);
StringStorage* ss_removeLen(const StringStorage* storage, int startIndex, int count, StringStorageAllocator allocator);
StringStorage* ss_replace(const StringStorage* storage, const StringStorage* oldValue, const StringStorage* newValue, StringStorageAllocator allocator);
StringStorage* ss_replaceChar(const StringStorage* storage, char oldChar, char newChar, StringStorageAllocator allocator);

// Case conversion (ASCII only)
StringStorage* ss_toLower(const StringStorage* storage, StringStorageAllocator allocator);
StringStorage* ss_toUpper(const StringStorage* storage, StringStorageAllocator allocator);

// Trimming (Unicode-aware)
StringStorage* ss_trim(const StringStorage* storage, StringStorageAllocator allocator);
StringStorage* ss_trimStart(const StringStorage* storage, StringStorageAllocator allocator);
StringStorage* ss_trimEnd(const StringStorage* storage, StringStorageAllocator allocator);

// Whitespace checking (Unicode-aware)
bool ss_isNullOrWhiteSpace(const StringStorage* storage);

// Splitting (caller must free returned array and its contents)
StringStorage** ss_split(const StringStorage* storage, char separator, int* count, StringStorageAllocator allocator);
StringStorage** ss_splitStr(const StringStorage* storage, const StringStorage* separator, int* count, StringStorageAllocator allocator);

// Hash computation
uint32_t ss_computeHash(const StringStorage* storage);
void ss_ensureHashComputed(StringStorage* storage);

// Internal helper functions
int ss_utf8CharCount(const char* str, int byteLen);
char ss_asciiToLower(char c);
char ss_asciiToUpper(char c);

// StringPool allocator (defined in StringPool.cpp)
void* stringpool_allocator(size_t size);

#ifdef __cplusplus
}
#endif
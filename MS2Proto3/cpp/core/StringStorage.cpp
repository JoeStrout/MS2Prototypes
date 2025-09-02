#include "StringStorage.h"
#include "StringPool.h"  // For defaultStringAllocator
#include <cstdlib>
#include <cstring>
#include <cctype>

// Static member definition
StringStorageAllocator StringStorage::allocator = StringPool::defaultStringAllocator;

// FNV-1a hash function
static uint32_t fnv1a_hash(const char* data, int len) {
    const uint32_t FNV_PRIME = 0x01000193;
    const uint32_t FNV_OFFSET_BASIS = 0x811c9dc5;
    
    uint32_t hash = FNV_OFFSET_BASIS;
    for (int i = 0; i < len; i++) {
        hash ^= (unsigned char)data[i];
        hash *= FNV_PRIME;
    }
    
    return hash == 0 ? 1 : hash;
}

// Static methods
StringStorage* StringStorage::create(const char* cstr) {
    if (!cstr) return nullptr;
    
    int len = strlen(cstr);
    StringStorage* storage = (StringStorage*)malloc(sizeof(StringStorage) + len + 1);
    if (!storage) return nullptr;
    
    storage->lenB = len;
    storage->lenC = utf8CharCount(cstr, len);
    storage->hash = fnv1a_hash(cstr, len);
    strcpy(storage->data, cstr);
    
    return storage;
}

StringStorage* StringStorage::createWithLength(int byteLen) {
    if (byteLen < 0) return nullptr;
    
    StringStorage* storage = (StringStorage*)malloc(sizeof(StringStorage) + byteLen + 1);
    if (!storage) return nullptr;
    
    storage->lenB = byteLen;
    storage->lenC = -1;  // Will be computed when needed
    storage->hash = 0;   // Will be computed when needed
    storage->data[byteLen] = '\0';  // Ensure null termination
    
    return storage;
}

void StringStorage::destroy(StringStorage* storage) {
    if (storage) {
        free(storage);
    }
}

// Helper methods
int StringStorage::utf8CharCount(const char* str, int byteLen) {
    int count = 0;
    for (int i = 0; i < byteLen; i++) {
        if ((str[i] & 0xC0) != 0x80) {
            count++;
        }
    }
    return count;
}

char StringStorage::asciiToLower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

char StringStorage::asciiToUpper(char c) {
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

// Instance methods
char StringStorage::charAt(int byteIndex) const {
    if (byteIndex < 0 || byteIndex >= lenB) return '\0';
    return data[byteIndex];
}

bool StringStorage::equals(const StringStorage* other) const {
    if (!other) return false;
    if (this == other) return true;
    if (lenB != other->lenB) return false;
    return memcmp(data, other->data, lenB) == 0;
}

int StringStorage::compare(const StringStorage* other) const {
    if (!other) return 1;
    if (this == other) return 0;
    
    int result = memcmp(data, other->data, lenB < other->lenB ? lenB : other->lenB);
    if (result == 0) {
        return lenB - other->lenB;
    }
    return result;
}

bool StringStorage::equalsIgnoreCase(const StringStorage* other) const {
    if (!other) return false;
    if (this == other) return true;
    if (lenB != other->lenB) return false;
    
    for (int i = 0; i < lenB; i++) {
        if (asciiToLower(data[i]) != asciiToLower(other->data[i])) {
            return false;
        }
    }
    return true;
}

int StringStorage::compareIgnoreCase(const StringStorage* other) const {
    if (!other) return 1;
    if (this == other) return 0;
    
    int minLen = lenB < other->lenB ? lenB : other->lenB;
    for (int i = 0; i < minLen; i++) {
        char c1 = asciiToLower(data[i]);
        char c2 = asciiToLower(other->data[i]);
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
    }
    return lenB - other->lenB;
}

int StringStorage::indexOf(const StringStorage* needle) const {
    return indexOf(needle, 0);
}

int StringStorage::indexOf(const StringStorage* needle, int startIndex) const {
    if (!needle) return -1;
    if (startIndex < 0 || startIndex >= lengthC()) return -1;
    if (needle->isEmpty()) return startIndex;
    
    // Convert character index to byte index
    int startByteIndex = UTF8CharIndexToByteIndex(
        (const unsigned char*)data, startIndex, lenB);
    if (startByteIndex < 0) return -1;
    
    const char* found = strstr(data + startByteIndex, needle->data);
    if (!found) return -1;
    
    // Convert back to character index
    int foundByteIndex = found - data;
    return UTF8ByteIndexToCharIndex(
        (const unsigned char*)data, foundByteIndex, lenB);
}

int StringStorage::indexOf(char ch) const {
    return indexOf(ch, 0);
}

int StringStorage::indexOf(char ch, int startIndex) const {
    if (startIndex < 0 || startIndex >= lengthC()) return -1;
    
    // Convert character index to byte index
    int startByteIndex = UTF8CharIndexToByteIndex(
        (const unsigned char*)data, startIndex, lenB);
    if (startByteIndex < 0) return -1;
    
    const char* found = strchr(data + startByteIndex, ch);
    if (!found) return -1;
    
    // Convert back to character index
    int foundByteIndex = found - data;
    return UTF8ByteIndexToCharIndex(
        (const unsigned char*)data, foundByteIndex, lenB);
}

int StringStorage::lastIndexOf(const StringStorage* needle) const {
    if (!needle || needle->isEmpty()) return -1;
    if (needle->lenB > lenB) return -1;
    
    // Search backwards
    for (int i = lenB - needle->lenB; i >= 0; i--) {
        if (memcmp(data + i, needle->data, needle->lenB) == 0) {
            return UTF8ByteIndexToCharIndex((const unsigned char*)data, i, lenB);
        }
    }
    return -1;
}

int StringStorage::lastIndexOf(char ch) const {
    const char* found = strrchr(data, ch);
    if (!found) return -1;
    
    int foundByteIndex = found - data;
    return UTF8ByteIndexToCharIndex((const unsigned char*)data, foundByteIndex, lenB);
}

bool StringStorage::contains(const StringStorage* needle) const {
    return indexOf(needle) >= 0;
}

bool StringStorage::startsWith(const StringStorage* prefix) const {
    if (!prefix) return false;
    if (prefix->lenB > lenB) return false;
    return memcmp(data, prefix->data, prefix->lenB) == 0;
}

bool StringStorage::endsWith(const StringStorage* suffix) const {
    if (!suffix) return false;
    if (suffix->lenB > lenB) return false;
    return memcmp(data + lenB - suffix->lenB, suffix->data, suffix->lenB) == 0;
}

StringStorage* StringStorage::substring(int startIndex) const {
    return substring(startIndex, lengthC() - startIndex);
}

StringStorage* StringStorage::substring(int startIndex, int length) const {
    if (startIndex < 0 || length < 0) return nullptr;
    if (startIndex >= lengthC()) return create("");
    
    // Convert character indices to byte indices
    int startByteIndex = UTF8CharIndexToByteIndex(
        (const unsigned char*)data, startIndex, lenB);
    if (startByteIndex < 0) return create("");
    
    int endCharIndex = startIndex + length;
    if (endCharIndex > lengthC()) endCharIndex = lengthC();
    int endByteIndex = UTF8CharIndexToByteIndex(
        (const unsigned char*)data, endCharIndex, lenB);
    if (endByteIndex < 0) endByteIndex = lenB;
    
    int subLenB = endByteIndex - startByteIndex;
    if (subLenB <= 0) return create("");
    
    StringStorage* result = createWithLength(subLenB);
    if (!result) return nullptr;
    
    memcpy(result->data, data + startByteIndex, subLenB);
    result->lenC = utf8CharCount(result->data, subLenB);
    result->hash = fnv1a_hash(result->data, subLenB);
    
    return result;
}

StringStorage* StringStorage::concat(const StringStorage* other) const {
    if (!other) return nullptr;
    
    int totalLen = lenB + other->lenB;
    StringStorage* result = createWithLength(totalLen);
    if (!result) return nullptr;
    
    memcpy(result->data, data, lenB);
    memcpy(result->data + lenB, other->data, other->lenB);
    result->lenC = lengthC() + other->lengthC();
    result->hash = fnv1a_hash(result->data, totalLen);
    
    return result;
}

StringStorage* StringStorage::toLower() const {
    StringStorage* result = createWithLength(lenB);
    if (!result) return nullptr;
    
    for (int i = 0; i < lenB; i++) {
        result->data[i] = asciiToLower(data[i]);
    }
    result->lenC = lenC;
    result->hash = fnv1a_hash(result->data, lenB);
    
    return result;
}

StringStorage* StringStorage::toUpper() const {
    StringStorage* result = createWithLength(lenB);
    if (!result) return nullptr;
    
    for (int i = 0; i < lenB; i++) {
        result->data[i] = asciiToUpper(data[i]);
    }
    result->lenC = lenC;
    result->hash = fnv1a_hash(result->data, lenB);
    
    return result;
}

StringStorage* StringStorage::trim() const {
    if (isEmpty()) return create("");
    
    unsigned char* start = (unsigned char*)data;
    unsigned char* end = start + lenB;
    
    // Find first non-whitespace character
    while (start < end) {
        unsigned char* next = start;
        unsigned long codePoint = UTF8DecodeAndAdvance(&next);
        if (!UnicodeCharIsWhitespace(codePoint)) break;
        start = next;
    }
    
    // Find last non-whitespace character
    unsigned char* lastNonWhite = start;
    unsigned char* current = start;
    while (current < end) {
        unsigned char* next = current;
        unsigned long codePoint = UTF8DecodeAndAdvance(&next);
        if (!UnicodeCharIsWhitespace(codePoint)) {
            lastNonWhite = next;
        }
        current = next;
    }
    
    if (start >= lastNonWhite) return create("");
    
    int trimmedLen = lastNonWhite - start;
    StringStorage* result = createWithLength(trimmedLen);
    if (!result) return nullptr;
    
    memcpy(result->data, start, trimmedLen);
    result->lenC = utf8CharCount(result->data, trimmedLen);
    result->hash = fnv1a_hash(result->data, trimmedLen);
    
    return result;
}

StringStorage* StringStorage::trimStart() const {
    if (isEmpty()) return create("");
    
    unsigned char* start = (unsigned char*)data;
    unsigned char* end = start + lenB;
    
    // Find first non-whitespace character
    while (start < end) {
        unsigned char* next = start;
        unsigned long codePoint = UTF8DecodeAndAdvance(&next);
        if (!UnicodeCharIsWhitespace(codePoint)) break;
        start = next;
    }
    
    int trimmedLen = end - start;
    if (trimmedLen <= 0) return create("");
    
    StringStorage* result = createWithLength(trimmedLen);
    if (!result) return nullptr;
    
    memcpy(result->data, start, trimmedLen);
    result->lenC = utf8CharCount(result->data, trimmedLen);
    result->hash = fnv1a_hash(result->data, trimmedLen);
    
    return result;
}

StringStorage* StringStorage::trimEnd() const {
    if (isEmpty()) return create("");
    
    unsigned char* start = (unsigned char*)data;
    unsigned char* end = start + lenB;
    
    // Find last non-whitespace character
    unsigned char* lastNonWhite = start;
    unsigned char* current = start;
    while (current < end) {
        unsigned char* next = current;
        unsigned long codePoint = UTF8DecodeAndAdvance(&next);
        if (!UnicodeCharIsWhitespace(codePoint)) {
            lastNonWhite = next;
        }
        current = next;
    }
    
    int trimmedLen = lastNonWhite - start;
    if (trimmedLen <= 0) return create("");
    
    StringStorage* result = createWithLength(trimmedLen);
    if (!result) return nullptr;
    
    memcpy(result->data, start, trimmedLen);
    result->lenC = utf8CharCount(result->data, trimmedLen);
    result->hash = fnv1a_hash(result->data, trimmedLen);
    
    return result;
}

bool StringStorage::isNullOrWhiteSpace() const {
    if (isEmpty()) return true;
    
    unsigned char* ptr = (unsigned char*)data;
    unsigned char* end = ptr + lenB;
    
    while (ptr < end) {
        unsigned long codePoint = UTF8DecodeAndAdvance(&ptr);
        if (!UnicodeCharIsWhitespace(codePoint)) {
            return false;
        }
    }
    return true;
}

StringStorage** StringStorage::split(char separator, int* count) const {
    *count = 0;
    if (isEmpty()) {
        StringStorage** result = (StringStorage**)malloc(sizeof(StringStorage*));
        result[0] = create("");
        *count = 1;
        return result;
    }
    
    // Count separators
    int sepCount = 0;
    for (int i = 0; i < lenB; i++) {
        if (data[i] == separator) sepCount++;
    }
    
    // Allocate array for results
    int resultCount = sepCount + 1;
    StringStorage** result = (StringStorage**)malloc(resultCount * sizeof(StringStorage*));
    
    // Split the string
    int resultIndex = 0;
    int start = 0;
    
    for (int i = 0; i <= lenB; i++) {
        if (i == lenB || data[i] == separator) {
            int tokenLen = i - start;
            StringStorage* token = createWithLength(tokenLen);
            if (token) {
                memcpy(token->data, data + start, tokenLen);
                token->lenC = utf8CharCount(token->data, tokenLen);
                token->hash = fnv1a_hash(token->data, tokenLen);
            }
            result[resultIndex] = token;
            resultIndex++;
            start = i + 1;
        }
    }
    
    *count = resultCount;
    return result;
}

uint32_t StringStorage::computeHash() const {
    return fnv1a_hash(data, lenB);
}

void StringStorage::ensureHashComputed() {
    if (hash == 0) {
        hash = computeHash();
    }
}

// Not yet implemented - more complex methods
StringStorage* StringStorage::insert(int startIndex, const StringStorage* value) const {
    // TODO: Implement
    (void)startIndex; (void)value;
    return nullptr;
}

StringStorage* StringStorage::remove(int startIndex) const {
    // TODO: Implement
    (void)startIndex;
    return nullptr;
}

StringStorage* StringStorage::remove(int startIndex, int count) const {
    // TODO: Implement
    (void)startIndex; (void)count;
    return nullptr;
}

StringStorage* StringStorage::replace(const StringStorage* oldValue, const StringStorage* newValue) const {
    // TODO: Implement
    (void)oldValue; (void)newValue;
    return nullptr;
}

StringStorage* StringStorage::replace(char oldChar, char newChar) const {
    // TODO: Implement
    (void)oldChar; (void)newChar;
    return nullptr;
}

StringStorage** StringStorage::split(const StringStorage* separator, int* count) const {
    // TODO: Implement
    (void)separator; (void)count;
    return nullptr;
}
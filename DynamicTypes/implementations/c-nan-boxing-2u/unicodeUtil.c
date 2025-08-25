// unicodeUtil.c
//
// Various utility functions for dealing with Unicode (especially UTF-8) text.
// Adapted from MiniScript's UnicodeUtil module for C.

#include "unicodeUtil.h"

// AdvanceUTF8
//
// Advance the character pointer within a UTF-8 buffer until it hits
// a limit defined by another character pointer, or has advanced the
// given number of characters.
void AdvanceUTF8(unsigned char **c, const unsigned char *maxc, int count)
{
    // UTF-8 characters always start with a byte whose high 2 bits
    // are not 10 (or put another way, subsequent bytes of a multi-byte
    // character sequence always have the high 2 bits set to 10).

    int charsFound = 0;
    while (charsFound < count && *c < maxc) {
        // advance to the end of the next character; that's at least one byte,
        // plus any intra-character bytes we might see
        (*c)++;
        while (IsUTF8IntraChar(**c) && *c < maxc) (*c)++;
        charsFound++;
    }
}

// BackupUTF8
//
// Back up the character pointer within a UTF-8 buffer until it hits
// a limit defined by another character pointer, or has backed up the
// given number of characters.
void BackupUTF8(unsigned char **c, const unsigned char *minc, int count)
{
    // UTF-8 characters always start with a byte whose high 2 bits
    // are not 10 (or put another way, subsequent bytes of a multi-byte
    // character sequence always have the high 2 bits set to 10).

    int charsFound = 0;
    while (charsFound < count && *c > minc) {
        // back up to the start of the previous character; that's at least one byte,
        // plus any intra-character bytes we might see
        (*c)--;
        while (IsUTF8IntraChar(**c) && *c > minc) (*c)--;
        charsFound++;
    }
}

// UTF8Encode
//
// Encode the given Unicode code point in UTF-8 form, followed by a
// null terminator. This requires up to 5 bytes.
int UTF8Encode(unsigned long uniChar, unsigned char *outBuf)
{
    unsigned char *c = outBuf;

    // There are four cases, depending on what range the code point is in.
    if (uniChar < 0x80) {
        *c = (unsigned char)uniChar;
        c++;
    }
    else if (uniChar < 0x800) {
        *c = ( 0xC0 | (unsigned char)(uniChar >> 6) );
        c++;
        *c = ( 0x80 | (unsigned char)(uniChar & 0x3F) );
        c++;
    }
    else if (uniChar < 0x10000) {
        *c = ( 0xE0 | (unsigned char)(uniChar >> 12) );
        c++;
        *c = ( 0x80 | ((unsigned char)(uniChar >> 6) & 0x3F) );
        c++;
        *c = ( 0x80 | (unsigned char)(uniChar & 0x3F) );
        c++;
    }
    else if (uniChar < 0x200000) {
        *c = ( 0xF0 | (unsigned char)(uniChar >> 18) );
        c++;
        *c = ( 0x80 | ((unsigned char)(uniChar >> 12) & 0x3F) );
        c++;
        *c = ( 0x80 | ((unsigned char)(uniChar >> 6) & 0x3F) );
        c++;
        *c = ( 0x80 | (unsigned char)(uniChar & 0x3F) );
        c++;
    }

    *c = 0;
    return c - outBuf;
}

// UTF8Decode
//
// Decode the first character of the given UTF-8 string back into its
// Unicode code point. This is the inverse of UTF8Encode.
unsigned long UTF8Decode(unsigned char *inBuf)
{
    unsigned char *c = inBuf;
    unsigned long out;
    
    // There are four cases, determined by the high bits of the first byte.
    if (0 == (*c & 0x80)) {
        out = *c;
    }
    else if (0xC0 == (*c & 0xE0)) {
        out = (c[0] & 0x1F);
        out = (out << 6) | (c[1] & 0x3F);
    }
    else if (0xE0 == (*c & 0xF0)) {
        out = (c[0] & 0x0F);
        out = (out << 6) | (c[1] & 0x3F);
        out = (out << 6) | (c[2] & 0x3F);
    }
    else {
        out = (c[0] & 0x07);
        out = (out << 6) | (c[1] & 0x3F);
        out = (out << 6) | (c[2] & 0x3F);
        out = (out << 6) | (c[3] & 0x3F);
    }
    
    return out;
}

// UTF8DecodeAndAdvance
//
// Decode the first character of the given UTF-8 string back into its
// Unicode code point, and advance the given pointer to the next character.
// This is like calling UTF8Decode followed by AdvanceUTF8, but is more
// efficient.
unsigned long UTF8DecodeAndAdvance(unsigned char **inBuf)
{
    // We have to copy the UTF8Decode code here... fortunately
    // it's not very long. We could combine them by having
    // UTF8Decode call this function, but then we would lose
    // some efficiency, and that is often important with UTF-8.

    unsigned char *c = *inBuf;
    unsigned long out;
    
    // There are four cases, determined by the high bits of the first byte.
    if (0 == (*c & 0x80)) {
        out = *c;
        *inBuf += 1;
    }
    else if (0xC0 == (*c & 0xE0)) {
        out = (c[0] & 0x1F);
        out = (out << 6) | (c[1] & 0x3F);
        *inBuf += 2;
    }
    else if (0xE0 == (*c & 0xF0)) {
        out = (c[0] & 0x0F);
        out = (out << 6) | (c[1] & 0x3F);
        out = (out << 6) | (c[2] & 0x3F);
        *inBuf += 3;
    }
    else {
        out = (c[0] & 0x07);
        out = (out << 6) | (c[1] & 0x3F);
        out = (out << 6) | (c[2] & 0x3F);
        out = (out << 6) | (c[3] & 0x3F);
        *inBuf += 4;
    }
    
    return out;
}

// UTF8CharacterCount
//
// Count the number of Unicode characters in a UTF-8 string.
int UTF8CharacterCount(const unsigned char *utf8String, int byteCount)
{
    if (!utf8String || byteCount <= 0) return 0;
    
    int charCount = 0;
    const unsigned char *ptr = utf8String;
    const unsigned char *end = utf8String + byteCount;
    
    while (ptr < end) {
        // Skip to next character start
        if (!IsUTF8IntraChar(*ptr)) {
            charCount++;
        }
        ptr++;
    }
    
    return charCount;
}

// UTF8ByteIndexToCharIndex
//
// Convert a byte index within a UTF-8 string to a character index.
int UTF8ByteIndexToCharIndex(const unsigned char *utf8String, int byteIndex, int maxBytes)
{
    if (!utf8String || byteIndex < 0 || byteIndex >= maxBytes) return -1;
    
    int charIndex = 0;
    const unsigned char *ptr = utf8String;
    
    for (int i = 0; i < byteIndex && i < maxBytes; i++) {
        if (!IsUTF8IntraChar(ptr[i])) {
            charIndex++;
        }
    }
    
    // If byteIndex points to an intra-character byte, it's invalid
    if (byteIndex < maxBytes && IsUTF8IntraChar(utf8String[byteIndex])) {
        return -1;
    }
    
    return charIndex;
}

// UTF8CharIndexToByteIndex
//
// Convert a character index within a UTF-8 string to a byte index.
int UTF8CharIndexToByteIndex(const unsigned char *utf8String, int charIndex, int maxBytes)
{
    if (!utf8String || charIndex < 0 || maxBytes <= 0) return -1;
    
    if (charIndex == 0) return 0;
    
    int currentCharIndex = 0;
    const unsigned char *ptr = utf8String;
    
    for (int i = 0; i < maxBytes; i++) {
        if (!IsUTF8IntraChar(ptr[i])) {
            if (currentCharIndex == charIndex) {
                return i;
            }
            currentCharIndex++;
        }
    }
    
    // If we've counted all characters and charIndex equals the count,
    // return the byte index at the end of the string
    if (currentCharIndex == charIndex) {
        return maxBytes;
    }
    
    return -1; // Character index out of range
}
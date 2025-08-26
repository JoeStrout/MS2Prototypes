// unicode.h
//
// Purpose: Unicode (in particular, UTF-8) utility functions: encoding
// and decoding characters, advancing through a UTF-8 string, etc.
// Dependencies: standard C libraries only

#ifndef UNICODE_H
#define UNICODE_H

#include <stdint.h>
#include <stdbool.h>

// Unicode utility functions
// Based on UnicodeUtil from MiniScript C++ implementation

// IsUTF8IntraChar
// Return whether this is an intra-character byte (in UTF-8 encoding),
// i.e., it's not the first or only byte of a character, but some subsequent
// byte of a multi-byte character.
static inline bool IsUTF8IntraChar(const unsigned char charByte)
{
    // It's an intra-char character if its high 2 bits are set to 10.
    return 0x80 == (charByte & 0xC0);
}

// AdvanceUTF8
// Advance the character pointer within a UTF-8 buffer until it hits
// a limit defined by another character pointer, or has advanced the
// given number of characters.
void AdvanceUTF8(unsigned char **c, const unsigned char *maxc, int count);

// BackupUTF8
// Back up the character pointer within a UTF-8 buffer until it hits
// a limit defined by another character pointer, or has backed up the
// given number of characters.
void BackupUTF8(unsigned char **c, const unsigned char *minc, int count);

// UTF8Encode
// Convert a Unicode code point to UTF-8 bytes
// Returns number of bytes written
int UTF8Encode(unsigned long uniChar, unsigned char *outBuf);

// UTF8Decode
// Get the next Unicode code point from UTF-8 string
// Returns the code point
unsigned long UTF8Decode(unsigned char *inBuf);

// UTF8DecodeAndAdvance
// Decode the first character of the given UTF-8 string back into its
// Unicode code point, and advance the given pointer to the next character.
unsigned long UTF8DecodeAndAdvance(unsigned char **inBuf);

// UTF8CharacterCount
// Count the number of Unicode characters in a UTF-8 string
int UTF8CharacterCount(const unsigned char* utf8, int byteLen);

// UTF8CharacterLength
// Get the byte length of the UTF-8 character starting at the given position
int UTF8CharacterLength(const unsigned char* utf8);

// UTF8CharIndexToByteIndex
// Convert character index to byte index in UTF-8 string
int UTF8CharIndexToByteIndex(const unsigned char* utf8String, int strLenB, int charIndex);

// UTF8ByteIndexToCharIndex  
// Convert byte index to character index in UTF-8 string
int UTF8ByteIndexToCharIndex(const unsigned char* utf8String, int strLenB, int byteIndex);

#endif // UNICODE_H
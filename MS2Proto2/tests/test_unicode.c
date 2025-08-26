// test_unicode.c
//
// Unit tests for the unicode module
// Tests UTF-8 utility functions in isolation

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/types/unicode.h"

// Test helper macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("❌ FAIL: %s\n", message); \
            return 0; \
        } else { \
            printf("✅ PASS: %s\n", message); \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("\n--- Running %s ---\n", #test_func); \
        if (!test_func()) { \
            printf("Test %s FAILED!\n", #test_func); \
            return 1; \
        } \
    } while(0)

// Test IsUTF8IntraChar function
int test_IsUTF8IntraChar() {
    // ASCII characters (0xxxxxxx) are NOT intra-char
    TEST_ASSERT(!IsUTF8IntraChar(0x41), "ASCII 'A' is not intra-char");
    TEST_ASSERT(!IsUTF8IntraChar(0x7F), "ASCII DEL is not intra-char");
    TEST_ASSERT(!IsUTF8IntraChar(0x00), "ASCII NUL is not intra-char");
    
    // Intra-char bytes (10xxxxxx) ARE intra-char
    TEST_ASSERT(IsUTF8IntraChar(0x80), "0x80 is intra-char");
    TEST_ASSERT(IsUTF8IntraChar(0xBF), "0xBF is intra-char");
    TEST_ASSERT(IsUTF8IntraChar(0xA0), "0xA0 is intra-char");
    
    // Start-of-multibyte chars (11xxxxxx) are NOT intra-char
    TEST_ASSERT(!IsUTF8IntraChar(0xC0), "0xC0 is not intra-char");
    TEST_ASSERT(!IsUTF8IntraChar(0xFF), "0xFF is not intra-char");
    TEST_ASSERT(!IsUTF8IntraChar(0xE0), "0xE0 is not intra-char");
    
    return 1;
}

// Test UTF8CharacterCount function
int test_UTF8CharacterCount() {
    // ASCII string
    const char* ascii = "Hello";
    TEST_ASSERT(UTF8CharacterCount((const unsigned char*)ascii, strlen(ascii)) == 5, 
                "ASCII string has correct char count");
    
    // Empty string
    TEST_ASSERT(UTF8CharacterCount((const unsigned char*)"", 0) == 0, 
                "Empty string has 0 characters");
    
    // Mixed ASCII and UTF-8 (café = 4 chars, 5 bytes)
    const char* cafe = "café";
    TEST_ASSERT(UTF8CharacterCount((const unsigned char*)cafe, strlen(cafe)) == 4, 
                "Mixed UTF-8 string 'café' has 4 characters");
    
    // Pure UTF-8 string with emoji (👋 = 1 char, 4 bytes)
    const char* wave = "👋";
    TEST_ASSERT(UTF8CharacterCount((const unsigned char*)wave, strlen(wave)) == 1, 
                "Emoji '👋' counts as 1 character");
    
    return 1;
}

// Test UTF8CharacterLength function
int test_UTF8CharacterLength() {
    // ASCII character
    const char* ascii = "A";
    TEST_ASSERT(UTF8CharacterLength((const unsigned char*)ascii) == 1, 
                "ASCII character has length 1");
    
    // 2-byte UTF-8 character (é)
    const char* e_acute = "é";
    TEST_ASSERT(UTF8CharacterLength((const unsigned char*)e_acute) == 2, 
                "2-byte UTF-8 character has length 2");
    
    // 3-byte UTF-8 character (€)
    const char* euro = "€";
    TEST_ASSERT(UTF8CharacterLength((const unsigned char*)euro) == 3, 
                "3-byte UTF-8 character has length 3");
    
    // 4-byte UTF-8 character (👋)
    const char* wave = "👋";
    TEST_ASSERT(UTF8CharacterLength((const unsigned char*)wave) == 4, 
                "4-byte UTF-8 character has length 4");
    
    return 1;
}

// Test AdvanceUTF8 function
int test_AdvanceUTF8() {
    const char* test_str = "Hé👋lo";  // H=1, é=2, 👋=4, l=1, o=1 bytes
    const unsigned char* start = (const unsigned char*)test_str;
    const unsigned char* end = start + strlen(test_str);
    unsigned char* ptr = (unsigned char*)start;
    
    // Advance 1 character (should move past 'H')
    AdvanceUTF8(&ptr, end, 1);
    TEST_ASSERT(ptr == start + 1, "Advance 1 char moves past ASCII 'H'");
    
    // Advance 1 more character (should move past 'é')
    AdvanceUTF8(&ptr, end, 1);
    TEST_ASSERT(ptr == start + 3, "Advance 1 char moves past 2-byte 'é'");
    
    // Advance 1 more character (should move past '👋')
    AdvanceUTF8(&ptr, end, 1);
    TEST_ASSERT(ptr == start + 7, "Advance 1 char moves past 4-byte '👋'");
    
    return 1;
}

// Test BackupUTF8 function
int test_BackupUTF8() {
    const char* test_str = "Hé👋lo";  // H=1, é=2, 👋=4, l=1, o=1 bytes
    const unsigned char* start = (const unsigned char*)test_str;
    const unsigned char* end = start + strlen(test_str);
    unsigned char* ptr = (unsigned char*)end;  // Start at the end
    
    // Back up 1 character (should move before 'o')
    BackupUTF8(&ptr, start, 1);
    TEST_ASSERT(ptr == end - 1, "Backup 1 char moves before ASCII 'o'");
    
    // Back up 1 more character (should move before 'l')
    BackupUTF8(&ptr, start, 1);
    TEST_ASSERT(ptr == end - 2, "Backup 1 char moves before ASCII 'l'");
    
    // Back up 1 more character (should move before '👋')
    BackupUTF8(&ptr, start, 1);
    TEST_ASSERT(ptr == start + 3, "Backup 1 char moves before 4-byte '👋'");
    
    return 1;
}

// Test UTF8Encode function
int test_UTF8Encode() {
    unsigned char buffer[8];
    int bytes_written;
    
    // Test ASCII character 'A' (U+0041)
    bytes_written = UTF8Encode(0x41, buffer);
    TEST_ASSERT(bytes_written == 1, "ASCII 'A' encodes to 1 byte");
    TEST_ASSERT(buffer[0] == 0x41, "ASCII 'A' encodes correctly");
    
    // Test 2-byte character 'é' (U+00E9)
    bytes_written = UTF8Encode(0xE9, buffer);
    TEST_ASSERT(bytes_written == 2, "Character 'é' encodes to 2 bytes");
    TEST_ASSERT(buffer[0] == 0xC3 && buffer[1] == 0xA9, "Character 'é' encodes correctly");
    
    return 1;
}

// Test UTF8Decode function
int test_UTF8Decode() {
    // Test ASCII character 'A'
    unsigned char ascii_buf[] = {0x41, 0x00};
    unsigned long decoded = UTF8Decode(ascii_buf);
    TEST_ASSERT(decoded == 0x41, "ASCII 'A' decodes correctly");
    
    // Test 2-byte character 'é' (0xC3 0xA9)
    unsigned char utf8_buf[] = {0xC3, 0xA9, 0x00};
    decoded = UTF8Decode(utf8_buf);
    TEST_ASSERT(decoded == 0xE9, "2-byte 'é' decodes correctly");
    
    return 1;
}

// Test CharIndexToByteIndex function
int test_CharIndexToByteIndex() {
    const char* test_str = "Hé👋lo";  // H=1, é=2, 👋=4, l=1, o=1 = 9 bytes total
    const unsigned char* utf8 = (const unsigned char*)test_str;
    int str_len = strlen(test_str);
    
    // Character 0 should be at byte 0
    TEST_ASSERT(UTF8CharIndexToByteIndex(utf8, str_len, 0) == 0, "Char 0 is at byte 0");
    
    // Character 1 should be at byte 1 (after 'H')
    TEST_ASSERT(UTF8CharIndexToByteIndex(utf8, str_len, 1) == 1, "Char 1 is at byte 1");
    
    // Character 2 should be at byte 3 (after 'é')
    TEST_ASSERT(UTF8CharIndexToByteIndex(utf8, str_len, 2) == 3, "Char 2 is at byte 3");
    
    // Character 3 should be at byte 7 (after '👋')
    TEST_ASSERT(UTF8CharIndexToByteIndex(utf8, str_len, 3) == 7, "Char 3 is at byte 7");
    
    return 1;
}

// Test ByteIndexToCharIndex function
int test_ByteIndexToCharIndex() {
    const char* test_str = "Hé👋lo";  // H=1, é=2, 👋=4, l=1, o=1 = 9 bytes total
    const unsigned char* utf8 = (const unsigned char*)test_str;
    int str_len = strlen(test_str);
    
    // Byte 0 should be character 0
    TEST_ASSERT(UTF8ByteIndexToCharIndex(utf8, str_len, 0) == 0, "Byte 0 is char 0");
    
    // Byte 1 should be character 1
    TEST_ASSERT(UTF8ByteIndexToCharIndex(utf8, str_len, 1) == 1, "Byte 1 is char 1");
    
    // Byte 3 should be character 2
    TEST_ASSERT(UTF8ByteIndexToCharIndex(utf8, str_len, 3) == 2, "Byte 3 is char 2");
    
    // Byte 7 should be character 3
    TEST_ASSERT(UTF8ByteIndexToCharIndex(utf8, str_len, 7) == 3, "Byte 7 is char 3");
    
    return 1;
}

int main() {
    printf("Unicode Module Test Suite\n");
    printf("========================\n");
    
    RUN_TEST(test_IsUTF8IntraChar);
    RUN_TEST(test_UTF8CharacterCount);
    RUN_TEST(test_UTF8CharacterLength);
    RUN_TEST(test_AdvanceUTF8);
    RUN_TEST(test_BackupUTF8);
    RUN_TEST(test_UTF8Encode);
    RUN_TEST(test_UTF8Decode);
    RUN_TEST(test_CharIndexToByteIndex);
    RUN_TEST(test_ByteIndexToCharIndex);
    
    printf("\n🎉 All Unicode tests passed!\n");
    return 0;
}
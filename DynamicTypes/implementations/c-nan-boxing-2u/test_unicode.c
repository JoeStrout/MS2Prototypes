#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "nanbox.h"
#include "nanbox_gc.h"

void test_utf8_basics() {
    printf("Testing UTF-8 basic functions...\n");
    
    // Test UTF-8 encoding/decoding
    unsigned char buf[5];
    
    // Test single byte (ASCII)
    int len = UTF8Encode('A', buf);
    assert(len == 1);
    assert(buf[0] == 'A' && buf[1] == 0);
    assert(UTF8Decode(buf) == 'A');
    
    // Test copyright symbol (2 bytes: ©)
    len = UTF8Encode(0xA9, buf);
    assert(len == 2);
    assert(UTF8Decode(buf) == 0xA9);
    
    // Test Euro symbol (3 bytes: €)
    len = UTF8Encode(0x20AC, buf);
    assert(len == 3);
    assert(UTF8Decode(buf) == 0x20AC);
    
    // Test emoji (4 bytes: 😀)
    len = UTF8Encode(0x1F600, buf);
    assert(len == 4);
    assert(UTF8Decode(buf) == 0x1F600);
    
    printf("✓ UTF-8 basic functions tests passed\n");
}

void test_character_counting() {
    printf("Testing UTF-8 character counting...\n");
    
    // ASCII string
    Value ascii = make_string("Hello");
    assert(string_lengthB(ascii) == 5);  // 5 bytes
    assert(string_length(ascii) == 5);   // 5 characters
    
    // Mixed ASCII + Unicode
    Value mixed = make_string("Hello © 2023");  // Hello + space + © + space + 2023
    assert(string_lengthB(mixed) == 13);  // "Hello" (5) + " " (1) + "©" (2) + " 2023" (5) = 13 bytes
    assert(string_length(mixed) == 12);   // 12 characters
    
    // All Unicode
    Value unicode = make_string("Café");  // Ca + fé (é is 2 bytes in UTF-8)
    assert(string_lengthB(unicode) == 5); // "Caf" (3) + "é" (2) = 5 bytes
    assert(string_length(unicode) == 4);  // 4 characters
    
    // Emoji
    Value emoji = make_string("Hi😀!");  // H + i + 😀 + !
    assert(string_lengthB(emoji) == 7);   // "Hi" (2) + "😀" (4) + "!" (1) = 7 bytes
    assert(string_length(emoji) == 4);    // 4 characters
    
    printf("✓ UTF-8 character counting tests passed\n");
}

void test_string_operations_unicode() {
	GC_PUSH_SCOPE();
    printf("Testing string operations with Unicode...\n");
    
	GC_LOCALS(str1, str2, str3, hello, world, result);

    // Test string equality with Unicode
    str1 = make_string("Café");
    str2 = make_string("Café");
    str3 = make_string("cafe");  // Different
    
    assert(string_equals(str1, str2));
    assert(!string_equals(str1, str3));
    
    // Test concatenation with Unicode
    hello = make_string("Hello ");
    world = make_string("🌍");  // Earth globe emoji
    result = string_concat(hello, world);
    
    assert(string_lengthB(result) == 10);  // "Hello " (6) + "🌍" (4) = 10 bytes
    assert(string_length(result) == 7);    // 7 characters
    
    // Verify the concatenated string
    const char* resultStr = as_cstring(result);
    assert(strcmp(resultStr, "Hello 🌍") == 0);
    
    printf("✓ String operations with Unicode tests passed\n");
	GC_POP_SCOPE();
}

void test_string_split_unicode() {
    printf("Testing string split with Unicode...\n");
    
    GC_PUSH_SCOPE();
    GC_LOCALS(unicode_str, empty_delim, char_list, char0, char1, char2, char3, phrase, copyright_delim, word_list);
    
    // Split Unicode string into characters
    unicode_str = make_string("A©€😀");  // 1+2+3+4 = 10 bytes, 4 characters
    empty_delim = make_string("");
    char_list = string_split(unicode_str, empty_delim);
    
    assert(list_count(char_list) == 4);  // Should have 4 characters
    
    // Check each character
    char0 = list_get(char_list, 0);
    char1 = list_get(char_list, 1);
    char2 = list_get(char_list, 2);
    char3 = list_get(char_list, 3);
    
    assert(strcmp(as_cstring(char0), "A") == 0);
    assert(strcmp(as_cstring(char1), "©") == 0);
    assert(strcmp(as_cstring(char2), "€") == 0);
    assert(strcmp(as_cstring(char3), "😀") == 0);
    
    // Test splitting on Unicode delimiter
    phrase = make_string("Hello©World©Test");
    copyright_delim = make_string("©");
    word_list = string_split(phrase, copyright_delim);
    
    assert(list_count(word_list) == 3);
    assert(strcmp(as_cstring(list_get(word_list, 0)), "Hello") == 0);
    assert(strcmp(as_cstring(list_get(word_list, 1)), "World") == 0);
    assert(strcmp(as_cstring(list_get(word_list, 2)), "Test") == 0);
    
    GC_POP_SCOPE();
    printf("✓ String split with Unicode tests passed\n");
}

void test_string_indexof_unicode() {
    printf("Testing string indexOf with Unicode...\n");
    
    GC_PUSH_SCOPE();
    GC_LOCALS(unicode_str, copyright, emoji, world, notfound);
    
    // Test finding Unicode characters
    unicode_str = make_string("Hello © World 😀");  
    copyright = make_string("©");
    emoji = make_string("😀");
    world = make_string("World");
    notfound = make_string("xyz");
    
    // indexOf should return CHARACTER positions, not byte positions
    int copyright_pos = string_indexOf(unicode_str, copyright);
    int emoji_pos = string_indexOf(unicode_str, emoji);
    int world_pos = string_indexOf(unicode_str, world);
    int notfound_pos = string_indexOf(unicode_str, notfound);
    
    assert(copyright_pos == 6);   // "Hello " = 6 characters, © is at position 6
    assert(world_pos == 8);       // "Hello © " = 8 characters, "World" starts at 8
    assert(emoji_pos == 14);      // "Hello © World " = 14 characters, 😀 is at position 14
    assert(notfound_pos == -1);
    
    GC_POP_SCOPE();
    printf("✓ String indexOf with Unicode tests passed\n");
}

void test_string_replace_unicode() {
    printf("Testing string replace with Unicode...\n");
    
	GC_PUSH_SCOPE();
	GC_LOCALS(original, arrow, and_word, replaced, emoji_str, happy, love, emoji_replaced);

    // Replace Unicode characters
    original = make_string("café → café");  // café arrow café
    arrow = make_string(" →");
    and_word = make_string(" and");
    replaced = string_replace(original, arrow, and_word);
    
	const char* replaced_cstr = as_cstring(replaced);
    assert(strcmp(replaced_cstr, "café and café") == 0);
    
    // Replace emoji
    emoji_str = make_string("I 😀 programming!");
    happy = make_string("😀");
    love = make_string("❤️");
    emoji_replaced = string_replace(emoji_str, happy, love);
    
    assert(strcmp(as_cstring(emoji_replaced), "I ❤️ programming!") == 0);
    
	GC_POP_SCOPE();
    printf("✓ String replace with Unicode tests passed\n");
}

void test_string_substring_unicode() {
    printf("Testing string substring with Unicode...\n");
    
    GC_PUSH_SCOPE();
    GC_LOCALS(unicode_str, chinese, emoji, hello, char_at_6, char_at_9);
    
    // Test substring with Unicode characters
    unicode_str = make_string("Hello 世界 😀!");  // Mixed ASCII, Chinese, emoji
    
    // Extract "世界" (characters 6-7)
    chinese = string_substring(unicode_str, 6, 2);
    assert(strcmp(as_cstring(chinese), "世界") == 0);
    
    // Extract "😀" (character 9)
    emoji = string_substring(unicode_str, 9, 1);
    assert(strcmp(as_cstring(emoji), "😀") == 0);
    
    // Extract "Hello" (characters 0-4)
    hello = string_substring(unicode_str, 0, 5);
    assert(strcmp(as_cstring(hello), "Hello") == 0);
    
    // Test charAt
    char_at_6 = string_charAt(unicode_str, 6);
    assert(strcmp(as_cstring(char_at_6), "世") == 0);
    
    char_at_9 = string_charAt(unicode_str, 9);
    assert(strcmp(as_cstring(char_at_9), "😀") == 0);
    
    GC_POP_SCOPE();
    printf("✓ String substring with Unicode tests passed\n");
}

void test_tiny_string_unicode() {
    printf("Testing tiny string optimization with Unicode...\n");
    
	GC_PUSH_SCOPE();
	GC_LOCALS(tiny_ascii, tiny_unicode, heap_unicode, tiny1, tiny2, concat);
	
    // ASCII string that fits in tiny string
    tiny_ascii = make_string("Hi");
    assert(is_tiny_string(tiny_ascii));
    assert(string_lengthB(tiny_ascii) == 2);
    assert(string_length(tiny_ascii) == 2);
    
    // Unicode string that fits in tiny string (5 bytes max)
    tiny_unicode = make_string("Café");  // 4 chars, 5 bytes (é is 2 bytes)
    assert(is_tiny_string(tiny_unicode));
    assert(string_lengthB(tiny_unicode) == 5);
    assert(string_length(tiny_unicode) == 4);
    
    // Unicode string that exceeds tiny string limit
    heap_unicode = make_string("Hello😀");  // 6 chars, 9 bytes (😀 is 4 bytes)
    assert(is_heap_string(heap_unicode));
    assert(string_lengthB(heap_unicode) == 9);
    assert(string_length(heap_unicode) == 6);
    
    // Test operations on tiny Unicode strings
    tiny1 = make_string("Hi©");  // 4 bytes
	assert(string_lengthB(tiny1) == 4);
    tiny2 = make_string("€!");   // 4 bytes
	assert(string_lengthB(tiny2) == 4);
    concat = string_concat(tiny1, tiny2);  // Should be heap string (7 bytes total)
    
    assert(is_heap_string(concat));
    assert(string_lengthB(concat) == 8);
    assert(string_length(concat) == 5);  // H + i + © + € + ! = 4 characters
    assert(strcmp(as_cstring(concat), "Hi©€!") == 0);
    
	GC_POP_SCOPE();
    printf("✓ Tiny string optimization with Unicode tests passed\n");
}

void test_index_conversion() {
    printf("Testing byte/character index conversion...\n");
 
    GC_PUSH_SCOPE();
    GC_LOCALS(mixed);

    mixed = make_string("A©€😀B");  // 1+2+3+4+1 = 11 bytes, 5 characters
    const char* str_data = as_cstring(mixed);
    int lenB = string_lengthB(mixed);
    
    // Test character index to byte index conversion
    assert(UTF8CharIndexToByteIndex((unsigned char*)str_data, 0, lenB) == 0);  // 'A'
    assert(UTF8CharIndexToByteIndex((unsigned char*)str_data, 1, lenB) == 1);  // '©' starts at byte 1
    assert(UTF8CharIndexToByteIndex((unsigned char*)str_data, 2, lenB) == 3);  // '€' starts at byte 3
    assert(UTF8CharIndexToByteIndex((unsigned char*)str_data, 3, lenB) == 6);  // '😀' starts at byte 6
    assert(UTF8CharIndexToByteIndex((unsigned char*)str_data, 4, lenB) == 10); // 'B' starts at byte 10
    assert(UTF8CharIndexToByteIndex((unsigned char*)str_data, 5, lenB) == 11); // End of string
    
    // Test byte index to character index conversion
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 0, lenB) == 0);   // Byte 0 -> char 0
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 1, lenB) == 1);   // Byte 1 -> char 1
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 3, lenB) == 2);   // Byte 3 -> char 2
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 6, lenB) == 3);   // Byte 6 -> char 3
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 10, lenB) == 4);  // Byte 10 -> char 4
    
    // Test invalid byte indexes (should return -1)
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 2, lenB) == -1);  // Middle of ©
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 4, lenB) == -1);  // Middle of €
    assert(UTF8ByteIndexToCharIndex((unsigned char*)str_data, 7, lenB) == -1);  // Middle of 😀
    
    GC_POP_SCOPE();
    printf("✓ Byte/character index conversion tests passed\n");
}

void test_comprehensive_example() {
    printf("Testing comprehensive Unicode example...\n");

    GC_PUSH_SCOPE();
    GC_LOCALS(greeting, space, words, word, rocket, plane, modified, substring, char_at_end);

    // Create a string with various Unicode content
    greeting = make_string("Hello 世界! Welcome to programming 🚀✨");
    
    printf("Original string: %s\n", as_cstring(greeting));
    printf("Byte length: %d\n", string_lengthB(greeting));
    printf("Character length: %d\n", string_length(greeting));
    
    // Split into words (space-separated)
    space = make_string(" ");
    words = string_split(greeting, space);
    int word_count = list_count(words);
    
    printf("Words (%d total):\n", word_count);
    for (int i = 0; i < word_count; i++) {
        word = list_get(words, i);
        printf("  [%d]: '%s' (%d chars, %d bytes)\n", 
               i, as_cstring(word), string_length(word), string_lengthB(word));
    }
    
    // Find and replace emoji
    rocket = make_string("🚀");
    plane = make_string("✈️");
    modified = string_replace(greeting, rocket, plane);
    
    printf("After replacing rocket with plane: %s\n", as_cstring(modified));
    
    // Extract substring
    substring = string_substring(greeting, 6, 2);  // Extract "世界"
    printf("Characters 6-7: '%s'\n", as_cstring(substring));
    
    // Character at specific position
    char_at_end = string_charAt(greeting, string_length(greeting) - 1);  // Last character
    printf("Last character: '%s'\n", as_cstring(char_at_end));
    
    GC_POP_SCOPE();
    printf("✓ Comprehensive Unicode example passed\n");
}

int main() {
    printf("Unicode Support Tests for NaN Boxing Implementation\n");
    printf("================================================\n\n");
    
    // Initialize GC
	gc_init();
    
    test_utf8_basics();
    test_character_counting();
    test_string_operations_unicode();
    test_string_split_unicode();
    test_string_indexof_unicode();
    test_string_replace_unicode();
    test_string_substring_unicode();
    test_tiny_string_unicode();
    test_index_conversion();
    test_comprehensive_example();
    
    // Clean up GC
	gc_shutdown();
	
    printf("\n🎉 All Unicode tests passed!\n");
    printf("The c-nan-boxing-2u implementation now has full UTF-8 Unicode support!\n");
    
    return 0;
}

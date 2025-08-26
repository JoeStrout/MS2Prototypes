#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../nanbox.h"

void test_list_creation() {
    printf("Testing list creation...\n");
    
    Value list = make_list(10);
    assert(is_list(list));
    assert(list_count(list) == 0);
    
    // Test default capacity
    Value list2 = make_list(0);
    assert(is_list(list2));
    assert(list_count(list2) == 0);
    
    printf("✓ List creation tests passed\n");
}

void test_list_add_get() {
    printf("Testing list add and get...\n");
    
    Value list = make_list(10);
    
    // Add integers
    list_add(list, make_int(42));
    list_add(list, make_int(100));
    list_add(list, make_int(-5));
    
    assert(list_count(list) == 3);
    
    // Test get operations
    Value item0 = list_get(list, 0);
    Value item1 = list_get(list, 1);
    Value item2 = list_get(list, 2);
    
    assert(is_int(item0) && as_int(item0) == 42);
    assert(is_int(item1) && as_int(item1) == 100);
    assert(is_int(item2) && as_int(item2) == -5);
    
    // Test out of bounds
    Value invalid = list_get(list, 10);
    assert(is_null(invalid));
    
    Value negative = list_get(list, -1);
    assert(is_null(negative));
    
    printf("✓ List add and get tests passed\n");
}

void test_list_set() {
    printf("Testing list set...\n");
    
    Value list = make_list(5);
    
    // Add some items first
    list_add(list, make_int(10));
    list_add(list, make_int(20));
    list_add(list, make_int(30));
    
    // Modify existing items
    list_set(list, 1, make_int(999));
    
    assert(as_int(list_get(list, 0)) == 10);
    assert(as_int(list_get(list, 1)) == 999);  // Modified
    assert(as_int(list_get(list, 2)) == 30);
    
    // Test out of bounds set (should be ignored)
    list_set(list, 10, make_int(123));
    assert(list_count(list) == 3); // Should still be 3
    
    printf("✓ List set tests passed\n");
}

void test_list_mixed_types() {
    printf("Testing list with mixed types...\n");
    
    Value list = make_list(10);
    
    // Add different types
    list_add(list, make_int(42));
    list_add(list, make_string("hello"));
    list_add(list, make_number(3.14));
    list_add(list, make_null());
    
    assert(list_count(list) == 4);
    
    // Verify types
    assert(is_int(list_get(list, 0)));
    assert(is_string(list_get(list, 1)));
    assert(is_number(list_get(list, 2)));
    assert(is_null(list_get(list, 3)));
    
    // Verify values
    assert(as_int(list_get(list, 0)) == 42);
    assert(strcmp(as_cstring(list_get(list, 1)), "hello") == 0);
    assert(as_number(list_get(list, 2)) == 3.14);
    
    printf("✓ Mixed types tests passed\n");
}

void test_list_indexOf() {
    printf("Testing list indexOf...\n");
    
    Value list = make_list(10);
    
    Value str1 = make_string("apple");
    Value str2 = make_string("banana");
    Value str3 = make_string("cherry");
    
    list_add(list, str1);
    list_add(list, make_int(100));
    list_add(list, str2);
    list_add(list, str3);
    
    // Test string searches
    Value search1 = make_string("banana");
    Value search2 = make_string("grape"); // Not in list
    
    assert(list_indexOf(list, search1) == 2);
    assert(list_indexOf(list, search2) == -1);
    
    // Test integer search
    Value int_search = make_int(100);
    assert(list_indexOf(list, int_search) == 1);
    
    Value int_missing = make_int(999);
    assert(list_indexOf(list, int_missing) == -1);
    
    printf("✓ List indexOf tests passed\n");
}

void test_string_split_chars() {
    printf("Testing string split (characters)...\n");
    
    Value str = make_string("abc");
    Value empty_delim = make_string("");
    Value list = string_split(str, empty_delim);
    
    assert(is_list(list));
    assert(list_count(list) == 3);
    
    Value item0 = list_get(list, 0);
    Value item1 = list_get(list, 1);
    Value item2 = list_get(list, 2);
    
    assert(is_string(item0) && strcmp(as_cstring(item0), "a") == 0);
    assert(is_string(item1) && strcmp(as_cstring(item1), "b") == 0);
    assert(is_string(item2) && strcmp(as_cstring(item2), "c") == 0);
    
    printf("✓ String split (characters) tests passed\n");
}

void test_string_split_words() {
    printf("Testing string split (words)...\n");
    
    Value str = make_string("hello world test");
    Value space_delim = make_string(" ");
    Value list = string_split(str, space_delim);
    
    assert(is_list(list));
    assert(list_count(list) == 3);
    
    Value item0 = list_get(list, 0);
    Value item1 = list_get(list, 1);
    Value item2 = list_get(list, 2);
    
    assert(is_string(item0) && strcmp(as_cstring(item0), "hello") == 0);
    assert(is_string(item1) && strcmp(as_cstring(item1), "world") == 0);
    assert(is_string(item2) && strcmp(as_cstring(item2), "test") == 0);
    
    printf("✓ String split (words) tests passed\n");
}

void test_list_capacity_bounds() {
    printf("Testing list capacity bounds...\n");
    
    Value small_list = make_list(2);
    
    // Fill to capacity
    list_add(small_list, make_int(1));
    list_add(small_list, make_int(2));
    assert(list_count(small_list) == 2);
    
    // Try to exceed capacity (should be ignored)
    list_add(small_list, make_int(3));
    assert(list_count(small_list) == 2); // Should still be 2
    
    // Verify existing items are intact
    assert(as_int(list_get(small_list, 0)) == 1);
    assert(as_int(list_get(small_list, 1)) == 2);
    
    printf("✓ List capacity bounds tests passed\n");
}

void test_benchmark_operations() {
    printf("Testing benchmark-specific operations...\n");
    
    // Test operations from numberWords benchmark
    Value singles_str = make_string(" one two three four five six seven eight nine ");
    Value space = make_string(" ");
    Value singles_list = string_split(singles_str, space);
    
    assert(is_list(singles_list));
    assert(list_count(singles_list) > 0);
    
    // Test indexOf on the list (like MiniScript's indexOf)
    Value search = make_string("five");
    int idx = list_indexOf(singles_list, search);
    assert(idx != -1);
    
    // Test that we can access items by index
    Value found_item = list_get(singles_list, idx);
    assert(is_string(found_item));
    assert(strcmp(as_cstring(found_item), "five") == 0);
    
    // Test levenshtein-style character split
    Value word = make_string("kitten");
    Value char_delim = make_string("");
    Value char_list = string_split(word, char_delim);
    
    assert(is_list(char_list));
    assert(list_count(char_list) == 6); // k-i-t-t-e-n
    
    Value first_char = list_get(char_list, 0);
    Value last_char = list_get(char_list, 5);
    
    assert(strcmp(as_cstring(first_char), "k") == 0);
    assert(strcmp(as_cstring(last_char), "n") == 0);
    
    printf("✓ Benchmark operations tests passed\n");
}

int main() {
    printf("NaN Boxing List Implementation Tests\n");
    printf("====================================\n\n");
    
    test_list_creation();
    test_list_add_get();
    test_list_set();
    test_list_mixed_types();
    test_list_indexOf();
    test_string_split_chars();
    test_string_split_words();
    test_list_capacity_bounds();
    test_benchmark_operations();
    
    printf("\n🎉 All list tests passed!\n");
    return 0;
}
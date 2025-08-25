#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../nanbox.h"

void test_string_creation() {
    printf("Testing string creation...\n");
    
    Value s = make_string("hello");
    assert(is_string(s));
    assert(string_length(s) == 5);
    assert(strcmp(as_cstring(s), "hello") == 0);
    
    Value empty = make_string("");
    assert(is_string(empty));
    assert(string_length(empty) == 0);
    assert(strcmp(as_cstring(empty), "") == 0);
    
    Value null_str = make_string(NULL);
    assert(is_nil(null_str));
    
    printf("✓ String creation tests passed\n");
}

void test_string_equality() {
    printf("Testing string equality...\n");
    
    Value s1 = make_string("hello");
    Value s2 = make_string("hello");
    Value s3 = make_string("world");
    
    assert(string_equals(s1, s2));
    assert(!string_equals(s1, s3));
    assert(values_equal(s1, s2));
    assert(!values_equal(s1, s3));
    
    printf("✓ String equality tests passed\n");
}

void test_string_concat() {
    printf("Testing string concatenation...\n");
    
    Value s1 = make_string("hello");
    Value s2 = make_string(" world");
    Value result = string_concat(s1, s2);
    
    assert(is_string(result));
    assert(string_length(result) == 11);
    assert(strcmp(as_cstring(result), "hello world") == 0);
    
    printf("✓ String concatenation tests passed\n");
}

void test_string_indexOf() {
    printf("Testing string indexOf...\n");
    
    Value haystack = make_string("hello world");
    Value needle1 = make_string("world");
    Value needle2 = make_string("foo");
    Value needle3 = make_string("o");
    
    assert(string_indexOf(haystack, needle1) == 6);
    assert(string_indexOf(haystack, needle2) == -1);
    assert(string_indexOf(haystack, needle3) == 4); // First 'o' in "hello"
    
    printf("✓ String indexOf tests passed\n");
}

void test_string_replace() {
    printf("Testing string replace...\n");
    
    // Test single replacement
    Value str = make_string("hello-world");
    Value from = make_string("-");
    Value to = make_string(" ");
    Value result = string_replace(str, from, to);
    
    assert(is_string(result));
    assert(strcmp(as_cstring(result), "hello world") == 0);
    
    // Test multiple replacements (this is the key new test)
    Value str_multi = make_string("thirty-seven and sixty-four");
    Value result_multi = string_replace(str_multi, from, to);
    assert(is_string(result_multi));
    assert(strcmp(as_cstring(result_multi), "thirty seven and sixty four") == 0);
    
    // Test no replacement
    Value str2 = make_string("hello world");
    Value from2 = make_string("foo");
    Value result2 = string_replace(str2, from2, to);
    assert(result2 == str2); // Should return original
    
    // Test empty string replacement (should return original)
    Value empty_from = make_string("");
    Value result3 = string_replace(str, empty_from, to);
    assert(result3 == str); // Should return original
    
    // Test replacement with longer string
    Value abc_str = make_string("a-b-c");
    Value dash = make_string("-");
    Value arrow = make_string(" -> ");
    Value abc_result = string_replace(abc_str, dash, arrow);
    assert(strcmp(as_cstring(abc_result), "a -> b -> c") == 0);
    
    printf("✓ String replace tests passed\n");
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

void test_string_split_empty_tokens() {
    printf("Testing string split with empty tokens...\n");
    
    // Test leading space (should create empty token)
    Value str_leading = make_string(" one two");
    Value space = make_string(" ");
    Value list_leading = string_split(str_leading, space);
    
    assert(is_list(list_leading));
    assert(list_count(list_leading) == 3);
    assert(strcmp(as_cstring(list_get(list_leading, 0)), "") == 0);  // Empty token
    assert(strcmp(as_cstring(list_get(list_leading, 1)), "one") == 0);
    assert(strcmp(as_cstring(list_get(list_leading, 2)), "two") == 0);
    
    // Test trailing space (should create empty token)
    Value str_trailing = make_string("one two ");
    Value list_trailing = string_split(str_trailing, space);
    
    assert(is_list(list_trailing));
    assert(list_count(list_trailing) == 3);
    assert(strcmp(as_cstring(list_get(list_trailing, 0)), "one") == 0);
    assert(strcmp(as_cstring(list_get(list_trailing, 1)), "two") == 0);
    assert(strcmp(as_cstring(list_get(list_trailing, 2)), "") == 0);  // Empty token
    
    // Test double spaces (should create empty token between)
    Value str_double = make_string("one  two");
    Value list_double = string_split(str_double, space);
    
    assert(is_list(list_double));
    assert(list_count(list_double) == 3);
    assert(strcmp(as_cstring(list_get(list_double, 0)), "one") == 0);
    assert(strcmp(as_cstring(list_get(list_double, 1)), "") == 0);    // Empty token
    assert(strcmp(as_cstring(list_get(list_double, 2)), "two") == 0);
    
    printf("✓ String split empty tokens tests passed\n");
}

void test_list_indexOf() {
    printf("Testing list indexOf...\n");
    
    Value str = make_string("one two three");
    Value space_delim = make_string(" ");
    Value list = string_split(str, space_delim);
    
    Value search1 = make_string("two");
    Value search2 = make_string("four");
    
    assert(list_indexOf(list, search1) == 1);
    assert(list_indexOf(list, search2) == -1);
    
    printf("✓ List indexOf tests passed\n");
}

void test_benchmark_operations() {
    printf("Testing benchmark-specific operations...\n");
    
    // Test operations from numberWords benchmark
    Value singles_str = make_string(" one two three four five six seven eight nine ");
    Value space = make_string(" ");
    Value singles_list = string_split(singles_str, space);
    
    // Should have non-empty strings (split should skip empty tokens)
    assert(list_count(singles_list) > 0);
    
    // Test indexOf on the list
    Value search = make_string("five");
    int idx = list_indexOf(singles_list, search);
    assert(idx != -1);
    
    // Test replace from numberWords
    Value hyphen_str = make_string("twenty-one");
    Value hyphen = make_string("-");
    Value space_repl = make_string(" ");
    Value replaced = string_replace(hyphen_str, hyphen, space_repl);
    assert(strcmp(as_cstring(replaced), "twenty one") == 0);
    
    printf("✓ Benchmark operations tests passed\n");
}

int main() {
    printf("NaN Boxing String Implementation Tests\n");
    printf("=====================================\n\n");
    
    test_string_creation();
    test_string_equality();
    test_string_concat();
    test_string_indexOf();
    test_string_replace();
    test_string_split_chars();
    test_string_split_words();
    test_string_split_empty_tokens();
    test_list_indexOf();
    test_benchmark_operations();
    
    printf("\n🎉 All tests passed!\n");
    return 0;
}
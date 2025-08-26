#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../nanbox.h"
#include "../nanbox_gc.h"

void test_string_creation() {
    GC_PUSH_SCOPE();
    printf("Testing string creation...\n");
    
    GC_LOCALS(s, empty, null_str);
    s = make_string("hello");
    assert(is_string(s));
    assert(string_length(s) == 5);
    assert(strcmp(as_cstring(s), "hello") == 0);
    
    empty = make_string("");
    assert(is_string(empty));
    assert(string_length(empty) == 0);
    assert(strcmp(as_cstring(empty), "") == 0);
    
    null_str = make_string(NULL);
    assert(is_null(null_str));
    
    printf("✓ String creation tests passed\n");
    GC_POP_SCOPE();
}

void test_string_equality() {
    GC_PUSH_SCOPE();
    printf("Testing string equality...\n");
    
    GC_LOCALS(s1, s2, s3);
    s1 = make_string("hello");
    s2 = make_string("hello");
    s3 = make_string("world");
    
    assert(string_equals(s1, s2));
    assert(!string_equals(s1, s3));
    assert(value_equal(s1, s2));
    assert(!value_equal(s1, s3));
    
    printf("✓ String equality tests passed\n");
    GC_POP_SCOPE();
}

void test_string_concat() {
    GC_PUSH_SCOPE();
    printf("Testing string concatenation...\n");
    
    GC_LOCALS(s1, s2, result);
    s1 = make_string("hello");
    s2 = make_string(" world");
    result = string_concat(s1, s2);
    
    assert(is_string(result));
    assert(string_length(result) == 11);
    assert(strcmp(as_cstring(result), "hello world") == 0);
    
    printf("✓ String concatenation tests passed\n");
    GC_POP_SCOPE();
}

void test_string_indexOf() {
    GC_PUSH_SCOPE();
    printf("Testing string indexOf...\n");
    
    GC_LOCALS(haystack, needle1, needle2, needle3);
    haystack = make_string("hello world");
    needle1 = make_string("world");
    needle2 = make_string("foo");
    needle3 = make_string("o");
    
    assert(string_indexOf(haystack, needle1) == 6);
    assert(string_indexOf(haystack, needle2) == -1);
    assert(string_indexOf(haystack, needle3) == 4); // First 'o' in "hello"
    
    printf("✓ String indexOf tests passed\n");
    GC_POP_SCOPE();
}

void test_string_replace() {
    GC_PUSH_SCOPE();
    printf("Testing string replace...\n");
    
    GC_LOCALS(str, from, to, result, str_multi, result_multi, str2, from2, result2, empty_from, result3, abc_str, dash, arrow, abc_result);
    
    // Test single replacement
    str = make_string("hello-world");
    from = make_string("-");
    to = make_string(" ");
    result = string_replace(str, from, to);
    
    assert(is_string(result));
    assert(strcmp(as_cstring(result), "hello world") == 0);
    
    // Test multiple replacements (this is the key new test)
    str_multi = make_string("thirty-seven and sixty-four");
    result_multi = string_replace(str_multi, from, to);
    assert(is_string(result_multi));
    assert(strcmp(as_cstring(result_multi), "thirty seven and sixty four") == 0);
    
    // Test no replacement
    str2 = make_string("hello world");
    from2 = make_string("foo");
    result2 = string_replace(str2, from2, to);
    assert(result2 == str2); // Should return original
    
    // Test empty string replacement (should return original)
    empty_from = make_string("");
    result3 = string_replace(str, empty_from, to);
    assert(result3 == str); // Should return original
    
    // Test replacement with longer string
    abc_str = make_string("a-b-c");
    dash = make_string("-");
    arrow = make_string(" -> ");
    abc_result = string_replace(abc_str, dash, arrow);
    assert(strcmp(as_cstring(abc_result), "a -> b -> c") == 0);
    
    printf("✓ String replace tests passed\n");
    GC_POP_SCOPE();
}

void test_string_split_chars() {
    GC_PUSH_SCOPE();
    printf("Testing string split (characters)...\n");
    
    GC_LOCALS(str, empty_delim, list, item0, item1, item2);
    str = make_string("abc");
    empty_delim = make_string("");
    list = string_split(str, empty_delim);
    
    assert(is_list(list));
    assert(list_count(list) == 3);
    
    item0 = list_get(list, 0);
    item1 = list_get(list, 1);
    item2 = list_get(list, 2);
    
    assert(is_string(item0) && strcmp(as_cstring(item0), "a") == 0);
    assert(is_string(item1) && strcmp(as_cstring(item1), "b") == 0);
    assert(is_string(item2) && strcmp(as_cstring(item2), "c") == 0);
    
    printf("✓ String split (characters) tests passed\n");
    GC_POP_SCOPE();
}

void test_string_split_words() {
    GC_PUSH_SCOPE();
    printf("Testing string split (words)...\n");
    
    GC_LOCALS(str, space_delim, list, item0, item1, item2);
    str = make_string("hello world test");
    space_delim = make_string(" ");
    list = string_split(str, space_delim);
    
    assert(is_list(list));
    assert(list_count(list) == 3);
    
    item0 = list_get(list, 0);
    item1 = list_get(list, 1);
    item2 = list_get(list, 2);
    
    assert(is_string(item0) && strcmp(as_cstring(item0), "hello") == 0);
    assert(is_string(item1) && strcmp(as_cstring(item1), "world") == 0);
    assert(is_string(item2) && strcmp(as_cstring(item2), "test") == 0);
    
    printf("✓ String split (words) tests passed\n");
    GC_POP_SCOPE();
}

void test_string_split_empty_tokens() {
    GC_PUSH_SCOPE();
    printf("Testing string split with empty tokens...\n");
    
    GC_LOCALS(str_leading, space, list_leading, str_trailing, list_trailing, str_double, list_double);
    
    // Test leading space (should create empty token)
    str_leading = make_string(" one two");
    space = make_string(" ");
    list_leading = string_split(str_leading, space);
    
    assert(is_list(list_leading));
    assert(list_count(list_leading) == 3);
    assert(strcmp(as_cstring(list_get(list_leading, 0)), "") == 0);  // Empty token
    assert(strcmp(as_cstring(list_get(list_leading, 1)), "one") == 0);
    assert(strcmp(as_cstring(list_get(list_leading, 2)), "two") == 0);
    
    // Test trailing space (should create empty token)
    str_trailing = make_string("one two ");
    list_trailing = string_split(str_trailing, space);
    
    assert(is_list(list_trailing));
    assert(list_count(list_trailing) == 3);
    assert(strcmp(as_cstring(list_get(list_trailing, 0)), "one") == 0);
    assert(strcmp(as_cstring(list_get(list_trailing, 1)), "two") == 0);
    assert(strcmp(as_cstring(list_get(list_trailing, 2)), "") == 0);  // Empty token
    
    // Test double spaces (should create empty token between)
    str_double = make_string("one  two");
    list_double = string_split(str_double, space);
    
    assert(is_list(list_double));
    assert(list_count(list_double) == 3);
    assert(strcmp(as_cstring(list_get(list_double, 0)), "one") == 0);
    assert(strcmp(as_cstring(list_get(list_double, 1)), "") == 0);    // Empty token
    assert(strcmp(as_cstring(list_get(list_double, 2)), "two") == 0);
    
    printf("✓ String split empty tokens tests passed\n");
    GC_POP_SCOPE();
}

void test_list_indexOf() {
    GC_PUSH_SCOPE();
    printf("Testing list indexOf...\n");
    
    GC_LOCALS(str, space_delim, list, search1, search2);
    str = make_string("one two three");
    space_delim = make_string(" ");
    list = string_split(str, space_delim);
    
    search1 = make_string("two");
    search2 = make_string("four");
    
    assert(list_indexOf(list, search1) == 1);
    assert(list_indexOf(list, search2) == -1);
    
    printf("✓ List indexOf tests passed\n");
    GC_POP_SCOPE();
}

void test_benchmark_operations() {
    GC_PUSH_SCOPE();
    printf("Testing benchmark-specific operations...\n");
    
    GC_LOCALS(singles_str, space, singles_list, search, hyphen_str, hyphen, space_repl, replaced);
    
    // Test operations from numberWords benchmark
    singles_str = make_string(" one two three four five six seven eight nine ");
    space = make_string(" ");
    singles_list = string_split(singles_str, space);
    
    // Should have non-empty strings (split should skip empty tokens)
    assert(list_count(singles_list) > 0);
    
    // Test indexOf on the list
    search = make_string("five");
    int idx = list_indexOf(singles_list, search);
    assert(idx != -1);
    
    // Test replace from numberWords
    hyphen_str = make_string("twenty-one");
    hyphen = make_string("-");
    space_repl = make_string(" ");
    replaced = string_replace(hyphen_str, hyphen, space_repl);
    assert(strcmp(as_cstring(replaced), "twenty one") == 0);
    
    printf("✓ Benchmark operations tests passed\n");
    GC_POP_SCOPE();
}

int main() {
    printf("NaN Boxing String Implementation Tests\n");
    printf("=====================================\n\n");
    
    // Initialize GC
    gc_init();
    
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
    
    // Clean up GC
    gc_shutdown();
    
    printf("\n🎉 All tests passed!\n");
    return 0;
}
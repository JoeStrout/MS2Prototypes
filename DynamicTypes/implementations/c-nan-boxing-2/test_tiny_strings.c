#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "nanbox.h"

// Simple gc_allocate implementation for testing
void* gc_allocate(size_t size) {
    return malloc(size);
}

void test_tiny_string_creation() {
    printf("Testing tiny string creation...\n");
    
    // Test strings within tiny string limit
    Value tiny1 = make_string("hello");
    Value tiny2 = make_string("a");
    Value tiny3 = make_string("12345"); // Exactly at limit
    
    assert(is_tiny_string(tiny1));
    assert(is_tiny_string(tiny2));
    assert(is_tiny_string(tiny3));
    
    printf("✓ Tiny string creation test passed\n");
}

void test_tiny_string_content() {
    printf("Testing tiny string content access...\n");
    
    Value str1 = make_string("hello");
    Value str2 = make_string("world");
    Value str3 = make_string("a");
    
    assert(strcmp(as_cstring(str1), "hello") == 0);
    assert(strcmp(as_cstring(str2), "world") == 0);
    assert(strcmp(as_cstring(str3), "a") == 0);
    
    assert(string_length(str1) == 5);
    assert(string_length(str2) == 5);
    assert(string_length(str3) == 1);
    
    printf("✓ Tiny string content test passed\n");
}

void test_heap_vs_tiny_strings() {
    printf("Testing heap vs tiny string selection...\n");
    
    Value tiny = make_string("short");   // Should be tiny
    Value heap = make_string("this is a very long string that exceeds the tiny string limit"); // Should be heap
    
    assert(is_tiny_string(tiny));
    assert(is_heap_string(heap));
    assert(!is_tiny_string(heap));
    assert(!is_heap_string(tiny));
    
    printf("✓ Heap vs tiny string selection test passed\n");
}

void test_string_operations() {
    printf("Testing string operations with tiny strings...\n");
    
    Value str1 = make_string("hello");
    Value str2 = make_string("world");
    
    // Test equality
    Value str1_copy = make_string("hello");
    assert(string_equals(str1, str1_copy));
    assert(!string_equals(str1, str2));
    
    // Test concatenation - result should be tiny if small enough
    Value concat = string_concat(make_string("hi"), make_string("!"));
    assert(is_tiny_string(concat));
    assert(strcmp(as_cstring(concat), "hi!") == 0);
    
    // Test concatenation that results in heap string
    Value long_concat = string_concat(str1, str2);
    assert(is_heap_string(long_concat)); // "helloworld" is 10 chars > 5
    assert(strcmp(as_cstring(long_concat), "helloworld") == 0);
    
    printf("✓ String operations test passed\n");
}

void test_string_replace() {
    printf("Testing string replace with tiny strings...\n");
    
    Value str = make_string("hello");
    Value from = make_string("l");
    Value to = make_string("L");
    
    Value result = string_replace(str, from, to);
    assert(strcmp(as_cstring(result), "heLLo") == 0);
    
    // Result should still be tiny
    assert(is_tiny_string(result));
    
    printf("✓ String replace test passed\n");
}

void test_mixed_tiny_heap() {
    printf("Testing mixed tiny/heap string operations...\n");
    
    Value tiny = make_string("hi");
    Value heap = make_string("this is a very long string");
    
    assert(is_tiny_string(tiny));
    assert(is_heap_string(heap));
    
    // Test equality between different types
    Value tiny2 = make_string("hi");
    assert(string_equals(tiny, tiny2));
    assert(!string_equals(tiny, heap));
    
    // Test concat of tiny + heap = heap
    Value mixed_concat = string_concat(tiny, heap);
    assert(is_heap_string(mixed_concat));
    
    printf("✓ Mixed tiny/heap operations test passed\n");
}

int main() {
    printf("Tiny String Tests\n");
    printf("=================\n\n");
    
    test_tiny_string_creation();
    test_tiny_string_content();
    test_heap_vs_tiny_strings();
    test_string_operations();
    test_string_replace();
    test_mixed_tiny_heap();
    
    printf("\n🎉 All tiny string tests passed!\n");
    return 0;
}
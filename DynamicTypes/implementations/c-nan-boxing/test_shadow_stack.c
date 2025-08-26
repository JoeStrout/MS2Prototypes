#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "nanbox_gc.h"
#include "gc.c"

void test_shadow_stack_basic() {
    printf("Testing basic shadow stack GC...\n");
    
    GC_PUSH_SCOPE();
    
    Value str1 = make_null();
    Value str2 = make_null();
    
    GC_PROTECT(&str1);
    GC_PROTECT(&str2);
    
    str1 = make_string("hello");
    str2 = make_string("world");
    
    // Test that strings are accessible
    assert(is_string(str1));
    assert(is_string(str2));
    assert(strcmp(as_cstring(str1), "hello") == 0);
    assert(strcmp(as_cstring(str2), "world") == 0);
    
    // Reassign str1 - this should be fine with shadow stack
    str1 = make_string("goodbye");
    
    assert(strcmp(as_cstring(str1), "goodbye") == 0);
    
    GC_POP_SCOPE();
    
    printf("✓ Basic shadow stack test passed\n");
}

void test_shadow_stack_collection() {
    printf("Testing shadow stack with collection...\n");
    
    GC_PUSH_SCOPE();
    
    Value list = make_null();
    GC_PROTECT(&list);
    
    list = make_list(10);
    
    // Add some strings to the list
    for (int i = 0; i < 5; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "item_%d", i);
        Value str = make_string(buffer);
        list_add(list, str);
    }
    
    // Force a collection
    gc_collect();
    
    // List should still be intact
    assert(is_list(list));
    assert(list_count(list) == 5);
    
    // Check that list items are still valid
    for (int i = 0; i < 5; i++) {
        Value item = list_get(list, i);
        assert(is_string(item));
        
        char expected[32];
        snprintf(expected, sizeof(expected), "item_%d", i);
        assert(strcmp(as_cstring(item), expected) == 0);
    }
    
    GC_POP_SCOPE();
    
    printf("✓ Shadow stack collection test passed\n");
}

void test_nested_scopes() {
    printf("Testing nested scopes...\n");
    
    GC_PUSH_SCOPE();
    
    Value outer_str = make_null();
    GC_PROTECT(&outer_str);
    outer_str = make_string("outer");
    
    {
        GC_PUSH_SCOPE();
        
        Value inner_str = make_null();
        GC_PROTECT(&inner_str);
        inner_str = make_string("inner");
        
        // Both should be accessible
        assert(strcmp(as_cstring(outer_str), "outer") == 0);
        assert(strcmp(as_cstring(inner_str), "inner") == 0);
        
        GC_POP_SCOPE();
    }
    
    // Outer should still be accessible
    assert(strcmp(as_cstring(outer_str), "outer") == 0);
    
    GC_POP_SCOPE();
    
    printf("✓ Nested scopes test passed\n");
}

int main() {
    printf("Shadow Stack GC Tests\n");
    printf("====================\n\n");
    
    gc_init();
    
    test_shadow_stack_basic();
    test_shadow_stack_collection();
    test_nested_scopes();
    
    printf("\n🎉 All shadow stack tests passed!\n");
    
    gc_shutdown();
    return 0;
}
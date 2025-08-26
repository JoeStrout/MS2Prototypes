// test_lists_capacity.c
//
// Additional tests for list capacity expansion functionality

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/types/lists.h"
#include "../include/types/nanbox.h"
#include "../include/types/strings.h"
#include "../include/types/gc.h"

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

// Test capacity expansion utilities
int test_capacity_expansion_utils() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(3);
    GC_PROTECT(&list);
    
    // Initially should not need expansion
    TEST_ASSERT(!list_needs_expansion(list), "Empty list doesn't need expansion");
    
    // Add items to capacity
    list_push(list, make_int(1));
    list_push(list, make_int(2));
    list_push(list, make_int(3));
    
    // Now should need expansion
    TEST_ASSERT(list_needs_expansion(list), "Full list needs expansion");
    
    // Create expanded list
    Value expanded = list_with_expanded_capacity(list);
    GC_PROTECT(&expanded);
    
    TEST_ASSERT(is_list(expanded), "Expanded result is a list");
    TEST_ASSERT(list_capacity(expanded) == 6, "Capacity doubled from 3 to 6");
    TEST_ASSERT(list_count(expanded) == 3, "All elements copied");
    TEST_ASSERT(!list_needs_expansion(expanded), "Expanded list doesn't need expansion");
    
    // Verify contents are the same
    for (int i = 0; i < 3; i++) {
        Value orig = list_get(list, i);
        Value exp = list_get(expanded, i);
        TEST_ASSERT(as_int(orig) == as_int(exp), "Element copied correctly");
    }
    
    // Original list should remain unchanged
    TEST_ASSERT(list_capacity(list) == 3, "Original list capacity unchanged");
    TEST_ASSERT(list_count(list) == 3, "Original list count unchanged");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test manual capacity expansion workflow
int test_manual_expansion_workflow() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(2);
    GC_PROTECT(&list);
    
    // Fill to capacity
    list_push(list, make_int(10));
    list_push(list, make_int(20));
    
    // Try to add beyond capacity (should be ignored)
    list_push(list, make_int(30));
    TEST_ASSERT(list_count(list) == 2, "Push beyond capacity ignored");
    
    // Manual expansion workflow
    if (list_needs_expansion(list)) {
        list = list_with_expanded_capacity(list);
        // Now we can add the item
        list_push(list, make_int(30));
    }
    
    TEST_ASSERT(list_count(list) == 3, "Item added after expansion");
    TEST_ASSERT(list_capacity(list) == 4, "Capacity expanded to 4");
    TEST_ASSERT(as_int(list_get(list, 2)) == 30, "New item stored correctly");
    
    // Add one more to verify space
    list_push(list, make_int(40));
    TEST_ASSERT(list_count(list) == 4, "Another item fits in expanded capacity");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test expansion with various data types
int test_expansion_with_mixed_types() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(3);
    Value str1 = make_string("hello");
    Value str2 = make_string("world");
    
    GC_PROTECT(&list);
    GC_PROTECT(&str1);
    GC_PROTECT(&str2);
    
    // Fill with mixed types
    list_push(list, make_int(42));
    list_push(list, str1);
    list_push(list, make_double(3.14));
    
    // Expand
    list = list_with_expanded_capacity(list);
    
    // Verify all types preserved
    TEST_ASSERT(is_int(list_get(list, 0)), "Int type preserved after expansion");
    TEST_ASSERT(is_string(list_get(list, 1)), "String type preserved after expansion");
    TEST_ASSERT(is_double(list_get(list, 2)), "Double type preserved after expansion");
    
    // Verify values preserved
    TEST_ASSERT(as_int(list_get(list, 0)) == 42, "Int value preserved");
    TEST_ASSERT(as_double(list_get(list, 2)) == 3.14, "Double value preserved");
    
    // Add more mixed types
    list_push(list, str2);
    list_push(list, make_nil());
    
    TEST_ASSERT(list_count(list) == 5, "More items added after expansion");
    TEST_ASSERT(is_string(list_get(list, 3)), "New string added");
    TEST_ASSERT(is_nil(list_get(list, 4)), "Nil added");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test edge cases
int test_expansion_edge_cases() {
    gc_init();
    GC_PUSH_SCOPE();
    
    // Test expansion of empty list
    Value empty = make_empty_list();
    GC_PROTECT(&empty);
    
    TEST_ASSERT(!list_needs_expansion(empty), "Empty list doesn't need expansion");
    
    Value expanded_empty = list_with_expanded_capacity(empty);
    GC_PROTECT(&expanded_empty);
    
    TEST_ASSERT(list_capacity(expanded_empty) == 16, "Empty list capacity doubled from 8 to 16");
    TEST_ASSERT(list_count(expanded_empty) == 0, "Expanded empty list still has count 0");
    
    // Test expansion of non-list
    Value expanded_nil = list_with_expanded_capacity(make_nil());
    TEST_ASSERT(is_nil(expanded_nil), "Expanding nil returns nil");
    
    Value expanded_int = list_with_expanded_capacity(make_int(42));
    TEST_ASSERT(is_nil(expanded_int), "Expanding non-list returns nil");
    
    // Test needs_expansion on non-list
    TEST_ASSERT(!list_needs_expansion(make_nil()), "Nil doesn't need expansion");
    TEST_ASSERT(!list_needs_expansion(make_int(42)), "Non-list doesn't need expansion");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test GC interaction with expansion
int test_expansion_gc_interaction() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(2);
    Value str = make_string("This is a test string for GC");
    
    GC_PROTECT(&list);
    GC_PROTECT(&str);
    
    list_push(list, str);
    list_push(list, make_int(100));
    
    // Expand the list
    list = list_with_expanded_capacity(list);
    
    // Force GC to test that both old and new lists, plus string, are handled correctly
    gc_collect();
    
    TEST_ASSERT(list_count(list) == 2, "List survives GC after expansion");
    TEST_ASSERT(is_string(list_get(list, 0)), "String in expanded list survives GC");
    TEST_ASSERT(is_int(list_get(list, 1)), "Int in expanded list survives GC");
    
    // Verify string content is still intact
    const char* content = as_cstring(list_get(list, 0));
    TEST_ASSERT(strlen(content) > 10, "String content intact after expansion and GC");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

int main() {
    printf("List Capacity Expansion Test Suite\n");
    printf("==================================\n");
    
    RUN_TEST(test_capacity_expansion_utils);
    RUN_TEST(test_manual_expansion_workflow);
    RUN_TEST(test_expansion_with_mixed_types);
    RUN_TEST(test_expansion_edge_cases);
    RUN_TEST(test_expansion_gc_interaction);
    
    printf("\n🎉 All capacity expansion tests passed!\n");
    printf("✨ List capacity expansion functionality implemented successfully\n");
    return 0;
}
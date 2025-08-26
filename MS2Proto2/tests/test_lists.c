// test_lists.c
//
// Unit tests for the lists module
// Tests dynamic array functionality with NaN-boxed Values

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/lists.h"
#include "../include/nanbox.h"
#include "../include/strings.h"
#include "../include/gc.h"

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

// Test list creation and basic properties
int test_list_creation() {
    gc_init();
    GC_PUSH_SCOPE();
    
    // Test make_list with specific capacity
    Value list1 = make_list(10);
    GC_PROTECT(&list1);
    
    TEST_ASSERT(is_list(list1), "make_list(10) creates list value");
    TEST_ASSERT(list_count(list1) == 0, "New list has count 0");
    TEST_ASSERT(list_capacity(list1) == 10, "List has requested capacity");
    
    // Test make_empty_list
    Value list2 = make_empty_list();
    GC_PROTECT(&list2);
    
    TEST_ASSERT(is_list(list2), "make_empty_list() creates list value");
    TEST_ASSERT(list_count(list2) == 0, "Empty list has count 0");
    TEST_ASSERT(list_capacity(list2) == 8, "Empty list has default capacity 8");
    
    // Test with zero/negative capacity
    Value list3 = make_list(0);
    Value list4 = make_list(-5);
    GC_PROTECT(&list3);
    GC_PROTECT(&list4);
    
    TEST_ASSERT(list_capacity(list3) == 8, "Zero capacity defaults to 8");
    TEST_ASSERT(list_capacity(list4) == 8, "Negative capacity defaults to 8");
    
    // Test as_list function
    List* list_ptr = as_list(list1);
    TEST_ASSERT(list_ptr != NULL, "as_list returns non-NULL for list");
    TEST_ASSERT(as_list(make_int(42)) == NULL, "as_list returns NULL for non-list");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test basic push and get operations
int test_list_push_get() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(5);
    GC_PROTECT(&list);
    
    // Push various types
    list_push(list, make_int(42));
    list_push(list, make_double(3.14));
    list_push(list, make_string("hello"));
    list_push(list, make_nil());
    
    TEST_ASSERT(list_count(list) == 4, "List count correct after pushes");
    
    // Test get operations
    Value v0 = list_get(list, 0);
    Value v1 = list_get(list, 1);
    Value v2 = list_get(list, 2);
    Value v3 = list_get(list, 3);
    
    TEST_ASSERT(is_int(v0) && as_int(v0) == 42, "First element correct");
    TEST_ASSERT(is_double(v1) && as_double(v1) == 3.14, "Second element correct");
    TEST_ASSERT(is_string(v2) && strcmp(as_cstring(v2), "hello") == 0, "Third element correct");
    TEST_ASSERT(is_nil(v3), "Fourth element correct");
    
    // Test out-of-bounds access
    Value invalid = list_get(list, 10);
    TEST_ASSERT(is_nil(invalid), "Out-of-bounds get returns nil");
    
    Value negative = list_get(list, -1);
    TEST_ASSERT(is_nil(negative), "Negative index get returns nil");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test list_set operations
int test_list_set() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(5);
    GC_PROTECT(&list);
    
    // Add some initial items
    list_push(list, make_int(1));
    list_push(list, make_int(2));
    list_push(list, make_int(3));
    
    // Test set operations
    list_set(list, 1, make_string("modified"));
    
    Value v0 = list_get(list, 0);
    Value v1 = list_get(list, 1);
    Value v2 = list_get(list, 2);
    
    TEST_ASSERT(is_int(v0) && as_int(v0) == 1, "First element unchanged");
    TEST_ASSERT(is_string(v1) && strcmp(as_cstring(v1), "modified") == 0, "Second element modified");
    TEST_ASSERT(is_int(v2) && as_int(v2) == 3, "Third element unchanged");
    
    // Test out-of-bounds set (should be ignored)
    list_set(list, 10, make_int(999));
    list_set(list, -1, make_int(999));
    TEST_ASSERT(list_count(list) == 3, "Count unchanged after invalid set");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test pop operations
int test_list_pop() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(5);
    GC_PROTECT(&list);
    
    // Add items to pop
    list_push(list, make_int(10));
    list_push(list, make_int(20));
    list_push(list, make_int(30));
    
    TEST_ASSERT(list_count(list) == 3, "Initial count correct");
    
    // Pop items
    Value popped1 = list_pop(list);
    TEST_ASSERT(is_int(popped1) && as_int(popped1) == 30, "Popped last item");
    TEST_ASSERT(list_count(list) == 2, "Count decreased after pop");
    
    Value popped2 = list_pop(list);
    TEST_ASSERT(is_int(popped2) && as_int(popped2) == 20, "Popped second-to-last item");
    
    Value popped3 = list_pop(list);
    TEST_ASSERT(is_int(popped3) && as_int(popped3) == 10, "Popped first item");
    TEST_ASSERT(list_count(list) == 0, "List empty after all pops");
    
    // Pop from empty list
    Value empty_pop = list_pop(list);
    TEST_ASSERT(is_nil(empty_pop), "Pop from empty list returns nil");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test insert operations
int test_list_insert() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(10);
    GC_PROTECT(&list);
    
    // Insert into empty list
    list_insert(list, 0, make_int(100));
    TEST_ASSERT(list_count(list) == 1, "Insert into empty list works");
    TEST_ASSERT(as_int(list_get(list, 0)) == 100, "Inserted element correct");
    
    // Insert at beginning
    list_insert(list, 0, make_int(50));
    TEST_ASSERT(list_count(list) == 2, "Insert at beginning works");
    TEST_ASSERT(as_int(list_get(list, 0)) == 50, "New first element correct");
    TEST_ASSERT(as_int(list_get(list, 1)) == 100, "Previous element shifted");
    
    // Insert at end
    list_insert(list, 2, make_int(200));
    TEST_ASSERT(list_count(list) == 3, "Insert at end works");
    TEST_ASSERT(as_int(list_get(list, 2)) == 200, "End element correct");
    
    // Insert in middle
    list_insert(list, 1, make_int(75));
    TEST_ASSERT(list_count(list) == 4, "Insert in middle works");
    TEST_ASSERT(as_int(list_get(list, 0)) == 50, "First element unchanged");
    TEST_ASSERT(as_int(list_get(list, 1)) == 75, "Inserted element correct");
    TEST_ASSERT(as_int(list_get(list, 2)) == 100, "Shifted element correct");
    TEST_ASSERT(as_int(list_get(list, 3)) == 200, "Last element unchanged");
    
    // Test invalid indices (should be ignored)
    int prev_count = list_count(list);
    list_insert(list, -1, make_int(999));
    list_insert(list, 100, make_int(999));
    TEST_ASSERT(list_count(list) == prev_count, "Invalid inserts ignored");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test remove operations
int test_list_remove() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(10);
    GC_PROTECT(&list);
    
    // Set up test data
    list_push(list, make_int(10));
    list_push(list, make_int(20));
    list_push(list, make_int(30));
    list_push(list, make_int(40));
    list_push(list, make_int(50));
    
    // Remove from middle
    list_remove(list, 2); // Remove 30
    TEST_ASSERT(list_count(list) == 4, "Remove from middle works");
    TEST_ASSERT(as_int(list_get(list, 0)) == 10, "First element unchanged");
    TEST_ASSERT(as_int(list_get(list, 1)) == 20, "Second element unchanged");
    TEST_ASSERT(as_int(list_get(list, 2)) == 40, "Third element shifted down");
    TEST_ASSERT(as_int(list_get(list, 3)) == 50, "Fourth element shifted down");
    
    // Remove from beginning
    list_remove(list, 0);
    TEST_ASSERT(list_count(list) == 3, "Remove from beginning works");
    TEST_ASSERT(as_int(list_get(list, 0)) == 20, "New first element correct");
    
    // Remove from end
    list_remove(list, 2);
    TEST_ASSERT(list_count(list) == 2, "Remove from end works");
    TEST_ASSERT(as_int(list_get(list, 1)) == 40, "Last element now correct");
    
    // Test invalid indices (should be ignored)
    int prev_count = list_count(list);
    list_remove(list, -1);
    list_remove(list, 100);
    TEST_ASSERT(list_count(list) == prev_count, "Invalid removes ignored");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test capacity limit behavior (TODO areas)
int test_capacity_limits() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(3); // Small capacity to test limits
    GC_PROTECT(&list);
    
    // Fill to capacity
    list_push(list, make_int(1));
    list_push(list, make_int(2));
    list_push(list, make_int(3));
    
    TEST_ASSERT(list_count(list) == 3, "List filled to capacity");
    
    // Try to push beyond capacity (should be ignored for now due to TODO)
    list_push(list, make_int(4));
    TEST_ASSERT(list_count(list) == 3, "Push beyond capacity ignored (TODO: capacity expansion)");
    
    // Try to insert beyond capacity
    list_insert(list, 1, make_int(99));
    TEST_ASSERT(list_count(list) == 3, "Insert beyond capacity ignored (TODO: capacity expansion)");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test search operations
int test_list_search() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(10);
    Value str1 = make_string("apple");
    Value str2 = make_string("banana");
    Value str3 = make_string("apple"); // Duplicate for testing
    
    GC_PROTECT(&list);
    GC_PROTECT(&str1);
    GC_PROTECT(&str2);
    GC_PROTECT(&str3);
    
    // Set up test data
    list_push(list, make_int(10));
    list_push(list, str1);
    list_push(list, make_double(3.14));
    list_push(list, str2);
    list_push(list, make_nil());
    list_push(list, str3); // Duplicate string
    
    // Test indexOf
    TEST_ASSERT(list_indexOf(list, make_int(10), 0) == 0, "Find int at beginning");
    TEST_ASSERT(list_indexOf(list, str1, 0) == 1, "Find first string");
    TEST_ASSERT(list_indexOf(list, make_double(3.14), 0) == 2, "Find double");
    TEST_ASSERT(list_indexOf(list, str2, 0) == 3, "Find second string");
    TEST_ASSERT(list_indexOf(list, make_nil(), 0) == 4, "Find nil");
    
    // Test indexOf with start position
    TEST_ASSERT(list_indexOf(list, str3, 2) == 5, "Find duplicate string with start pos");
    TEST_ASSERT(list_indexOf(list, make_int(10), 1) == -1, "Don't find int after start pos");
    
    // Test contains
    TEST_ASSERT(list_contains(list, make_int(10)) == true, "Contains int");
    TEST_ASSERT(list_contains(list, str1) == true, "Contains string");
    TEST_ASSERT(list_contains(list, make_int(999)) == false, "Doesn't contain non-existent int");
    TEST_ASSERT(list_contains(list, make_string("orange")) == false, "Doesn't contain non-existent string");
    
    // Test with empty list
    Value empty_list = make_empty_list();
    GC_PROTECT(&empty_list);
    TEST_ASSERT(list_indexOf(empty_list, make_int(1), 0) == -1, "IndexOf in empty list returns -1");
    TEST_ASSERT(list_contains(empty_list, make_int(1)) == false, "Empty list contains nothing");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test utility functions
int test_list_utilities() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value original = make_list(5);
    GC_PROTECT(&original);
    
    // Set up test data
    list_push(original, make_int(100));
    list_push(original, make_string("test"));
    list_push(original, make_double(2.5));
    
    // Test list_clear
    list_clear(original);
    TEST_ASSERT(list_count(original) == 0, "Clear empties the list");
    TEST_ASSERT(list_capacity(original) == 5, "Clear preserves capacity");
    
    // Repopulate for copy test
    list_push(original, make_int(1));
    list_push(original, make_int(2));
    list_push(original, make_int(3));
    
    // Test list_copy
    Value copy = list_copy(original);
    GC_PROTECT(&copy);
    
    TEST_ASSERT(is_list(copy), "Copy creates a list");
    TEST_ASSERT(list_count(copy) == 3, "Copy has same count");
    TEST_ASSERT(list_capacity(copy) == 5, "Copy has same capacity");
    
    // Verify contents are the same
    for (int i = 0; i < 3; i++) {
        Value orig_val = list_get(original, i);
        Value copy_val = list_get(copy, i);
        TEST_ASSERT(as_int(orig_val) == as_int(copy_val), "Copy contents match original");
    }
    
    // Verify they are independent
    list_push(original, make_int(4));
    TEST_ASSERT(list_count(original) == 4, "Original can be modified");
    TEST_ASSERT(list_count(copy) == 3, "Copy remains unchanged");
    
    // Test copy of empty list
    Value empty = make_empty_list();
    Value empty_copy = list_copy(empty);
    GC_PROTECT(&empty);
    GC_PROTECT(&empty_copy);
    
    TEST_ASSERT(is_list(empty_copy), "Empty list copy creates a list");
    TEST_ASSERT(list_count(empty_copy) == 0, "Empty list copy has count 0");
    
    // Test copy of nil (should return nil)
    Value nil_copy = list_copy(make_nil());
    TEST_ASSERT(is_nil(nil_copy), "Copy of non-list returns nil");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test mixed data types and edge cases
int test_mixed_types_edge_cases() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(10);
    GC_PROTECT(&list);
    
    // Test with various data types
    list_push(list, make_int(42));
    list_push(list, make_double(-3.14159));
    list_push(list, make_string("hello world"));
    list_push(list, make_nil());
    list_push(list, make_string("")); // Empty string
    
    Value nested_list = make_list(3);
    list_push(nested_list, make_int(1));
    list_push(nested_list, make_int(2));
    list_push(list, nested_list);
    
    GC_PROTECT(&nested_list);
    
    TEST_ASSERT(list_count(list) == 6, "Mixed types added correctly");
    
    // Verify each type
    TEST_ASSERT(is_int(list_get(list, 0)), "Int type preserved");
    TEST_ASSERT(is_double(list_get(list, 1)), "Double type preserved");
    TEST_ASSERT(is_string(list_get(list, 2)), "String type preserved");
    TEST_ASSERT(is_nil(list_get(list, 3)), "Nil type preserved");
    TEST_ASSERT(is_string(list_get(list, 4)), "Empty string type preserved");
    TEST_ASSERT(is_list(list_get(list, 5)), "List type preserved");
    
    // Test operations on invalid list values
    list_push(make_nil(), make_int(1)); // Should be ignored
    list_set(make_int(42), 0, make_int(1)); // Should be ignored
    Value invalid_get = list_get(make_string("not a list"), 0);
    TEST_ASSERT(is_nil(invalid_get), "Operations on non-lists handle gracefully");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test GC integration
int test_gc_integration() {
    gc_init();
    GC_PUSH_SCOPE();
    
    Value list = make_list(5);
    GC_PROTECT(&list);
    
    // Add strings that will need GC tracking
    Value str1 = make_string("This is a longer string that will be heap-allocated");
    Value str2 = make_string("Another heap string");
    
    list_push(list, str1);
    list_push(list, str2);
    list_push(list, make_int(42));
    
    // Force GC to test that list and strings survive
    gc_collect();
    
    TEST_ASSERT(list_count(list) == 3, "List survives GC");
    TEST_ASSERT(is_string(list_get(list, 0)), "First string survives GC");
    TEST_ASSERT(is_string(list_get(list, 1)), "Second string survives GC");
    TEST_ASSERT(is_int(list_get(list, 2)), "Int survives GC");
    
    // Test that list contents remain valid after GC
    const char* s1 = as_cstring(list_get(list, 0));
    const char* s2 = as_cstring(list_get(list, 1));
    TEST_ASSERT(strlen(s1) > 10, "First string content intact after GC");
    TEST_ASSERT(strlen(s2) > 10, "Second string content intact after GC");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

int main() {
    printf("Lists Module Test Suite\n");
    printf("======================\n");
    
    RUN_TEST(test_list_creation);
    RUN_TEST(test_list_push_get);
    RUN_TEST(test_list_set);
    RUN_TEST(test_list_pop);
    RUN_TEST(test_list_insert);
    RUN_TEST(test_list_remove);
    RUN_TEST(test_capacity_limits);
    RUN_TEST(test_list_search);
    RUN_TEST(test_list_utilities);
    RUN_TEST(test_mixed_types_edge_cases);
    RUN_TEST(test_gc_integration);
    
    printf("\n🎉 All lists tests passed!\n");
    printf("📝 Note: Capacity expansion TODOs identified for future implementation\n");
    return 0;
}
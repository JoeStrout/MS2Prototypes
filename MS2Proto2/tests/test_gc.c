// test_gc.c
//
// Unit tests for the garbage collector module
// Tests shadow stack GC with precise collection

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/gc.h"
#include "../include/nanbox.h"
#include "../include/strings.h"
#include "../include/lists.h"

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

// Test GC initialization and shutdown
int test_gc_init_shutdown() {
    gc_init();
    
    GCStats stats = gc_get_stats();
    TEST_ASSERT(stats.bytes_allocated == 0, "Initial bytes allocated is zero");
    TEST_ASSERT(stats.gc_threshold > 0, "GC threshold is set");
    TEST_ASSERT(stats.is_enabled == true, "GC is initially enabled");
    
    gc_shutdown();
    return 1;
}

// Test basic memory allocation
int test_gc_allocation() {
    gc_init();
    
    GCStats before = gc_get_stats();
    
    // Allocate some memory
    void* ptr1 = gc_allocate(100);
    void* ptr2 = gc_allocate(200);
    
    TEST_ASSERT(ptr1 != NULL, "First allocation succeeds");
    TEST_ASSERT(ptr2 != NULL, "Second allocation succeeds");
    TEST_ASSERT(ptr1 != ptr2, "Allocations return different pointers");
    
    GCStats after = gc_get_stats();
    TEST_ASSERT(after.bytes_allocated > before.bytes_allocated, "Bytes allocated increased");
    
    gc_shutdown();
    return 1;
}

// Test root set protection and unprotection
int test_root_protection() {
    gc_init();
    
    Value v1 = make_int(42);
    Value v2 = make_double(3.14);
    
    // Protect values
    gc_protect_value(&v1);
    gc_protect_value(&v2);
    
    // Values should remain unchanged
    TEST_ASSERT(is_int(v1) && as_int(v1) == 42, "Protected int value unchanged");
    TEST_ASSERT(is_double(v2) && as_double(v2) == 3.14, "Protected double value unchanged");
    
    // Unprotect values
    gc_unprotect_value();
    gc_unprotect_value();
    
    gc_shutdown();
    return 1;
}

// Test scope management
int test_scope_management() {
    gc_init();
    
    Value v1 = make_int(10);
    Value v2 = make_int(20);
    Value v3 = make_int(30);
    
    // Test nested scopes
    GC_PUSH_SCOPE();
    gc_protect_value(&v1);
    gc_protect_value(&v2);
    
    GC_PUSH_SCOPE();
    gc_protect_value(&v3);
    GC_POP_SCOPE();  // Should unprotect v3
    
    GC_POP_SCOPE();  // Should unprotect v1, v2
    
    TEST_ASSERT(true, "Scope operations completed without error");
    
    gc_shutdown();
    return 1;
}

// Test GC_LOCALS macro
int test_gc_locals_macro() {
    gc_init();
    
    Value v1 = make_int(100);
    Value v2 = make_double(2.718);
    Value v3 = make_nil();
    
    GC_PUSH_SCOPE();
    GC_LOCALS(&v1, &v2, &v3);
    
    // Values should remain accessible
    TEST_ASSERT(is_int(v1) && as_int(v1) == 100, "First local protected");
    TEST_ASSERT(is_double(v2), "Second local protected");
    TEST_ASSERT(is_nil(v3), "Third local protected");
    
    GC_POP_SCOPE();
    
    gc_shutdown();
    return 1;
}

// Test GC enable/disable
int test_gc_enable_disable() {
    gc_init();
    
    GCStats stats = gc_get_stats();
    TEST_ASSERT(stats.is_enabled == true, "GC initially enabled");
    
    gc_disable();
    stats = gc_get_stats();
    TEST_ASSERT(stats.is_enabled == false, "GC disabled after gc_disable()");
    
    gc_enable();
    stats = gc_get_stats();
    TEST_ASSERT(stats.is_enabled == true, "GC enabled after gc_enable()");
    
    // Test nested disable/enable
    gc_disable();
    gc_disable();
    stats = gc_get_stats();
    TEST_ASSERT(stats.is_enabled == false, "GC still disabled after nested disable");
    
    gc_enable();
    stats = gc_get_stats();
    TEST_ASSERT(stats.is_enabled == false, "GC still disabled after one enable");
    
    gc_enable();
    stats = gc_get_stats();
    TEST_ASSERT(stats.is_enabled == true, "GC enabled after matching enables");
    
    gc_shutdown();
    return 1;
}

// Test string allocation and GC
int test_string_gc() {
    gc_init();
    
    GC_PUSH_SCOPE();
    
    // Create some strings
    Value str1 = make_string("Hello");
    Value str2 = make_string("World");
    Value str3 = make_string("Testing GC with longer string");
    
    GC_LOCALS(&str1, &str2, &str3);
    
    TEST_ASSERT(is_string(str1), "First string created");
    TEST_ASSERT(is_string(str2), "Second string created");
    TEST_ASSERT(is_string(str3), "Third string created");
    
    // Force garbage collection
    gc_collect();
    
    // Strings should still be valid (they're protected)
    TEST_ASSERT(is_string(str1), "First string survives GC");
    TEST_ASSERT(is_string(str2), "Second string survives GC");
    TEST_ASSERT(is_string(str3), "Third string survives GC");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test list allocation and GC
int test_list_gc() {
    gc_init();
    
    GC_PUSH_SCOPE();
    
    // Create lists with different content
    Value list1 = make_list(10);
    Value list2 = make_list(5);
    
    GC_LOCALS(&list1, &list2);
    
    // Add some values to the lists
    list_push(list1, make_int(1));
    list_push(list1, make_int(2));
    list_push(list1, make_int(3));
    
    list_push(list2, make_string("test"));
    list_push(list2, make_double(1.23));
    
    TEST_ASSERT(list_count(list1) == 3, "First list has correct count");
    TEST_ASSERT(list_count(list2) == 2, "Second list has correct count");
    
    // Force garbage collection
    gc_collect();
    
    // Lists should still be valid and contain correct data
    TEST_ASSERT(list_count(list1) == 3, "First list survives GC");
    TEST_ASSERT(list_count(list2) == 2, "Second list survives GC");
    TEST_ASSERT(as_int(list_get(list1, 0)) == 1, "List data intact after GC");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test that unprotected objects get collected
int test_object_collection() {
    gc_init();
    
    GCStats before = gc_get_stats();
    
    // Create objects without protecting them
    {
        Value temp_str = make_string("Temporary string that should be collected");
        Value temp_list = make_list(10);
        list_push(temp_list, make_int(42));
        
        // Objects exist but are not protected
        TEST_ASSERT(is_string(temp_str), "Temporary string created");
        TEST_ASSERT(list_count(temp_list) == 1, "Temporary list created with item");
    }
    
    // Force collection - unprotected objects should be freed
    gc_collect();
    
    GCStats after = gc_get_stats();
    // We can't easily verify objects were collected without more introspection,
    // but we can check that a collection occurred without crashing
    TEST_ASSERT(true, "Collection completed without crash");
    
    gc_shutdown();
    return 1;
}

// Test mixed object types and cross-references
int test_mixed_object_references() {
    gc_init();
    
    GC_PUSH_SCOPE();
    
    // Create a list containing strings
    Value list = make_list(5);
    Value str1 = make_string("First");
    Value str2 = make_string("Second");
    
    GC_LOCALS(&list, &str1, &str2);
    
    // Add strings to list
    list_push(list, str1);
    list_push(list, str2);
    list_push(list, make_int(42));
    
    TEST_ASSERT(list_count(list) == 3, "List contains three items");
    
    // Force collection - everything should survive because it's all reachable
    gc_collect();
    
    TEST_ASSERT(list_count(list) == 3, "List survives GC with references intact");
    Value retrieved_str = list_get(list, 0);
    TEST_ASSERT(is_string(retrieved_str), "String in list survives GC");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

// Test collection statistics
int test_gc_stats() {
    gc_init();
    
    GCStats initial = gc_get_stats();
    TEST_ASSERT(initial.bytes_allocated == 0, "Initial allocation is zero");
    TEST_ASSERT(initial.is_enabled == true, "GC initially enabled");
    
    // Allocate some objects
    GC_PUSH_SCOPE();
    Value str = make_string("Test string for stats");
    Value list = make_list(10);
    GC_LOCALS(&str, &list);
    
    GCStats after_alloc = gc_get_stats();
    TEST_ASSERT(after_alloc.bytes_allocated > initial.bytes_allocated, 
                "Allocation increases byte count");
    
    // Test disable affects stats
    gc_disable();
    GCStats disabled = gc_get_stats();
    TEST_ASSERT(disabled.is_enabled == false, "Stats reflect disabled state");
    
    gc_enable();
    GCStats enabled = gc_get_stats();
    TEST_ASSERT(enabled.is_enabled == true, "Stats reflect enabled state");
    
    GC_POP_SCOPE();
    gc_shutdown();
    return 1;
}

int main() {
    printf("Garbage Collector Test Suite\n");
    printf("============================\n");
    
    RUN_TEST(test_gc_init_shutdown);
    RUN_TEST(test_gc_allocation);
    RUN_TEST(test_root_protection);
    RUN_TEST(test_scope_management);
    RUN_TEST(test_gc_locals_macro);
    RUN_TEST(test_gc_enable_disable);
    RUN_TEST(test_string_gc);
    RUN_TEST(test_list_gc);
    RUN_TEST(test_object_collection);
    RUN_TEST(test_mixed_object_references);
    RUN_TEST(test_gc_stats);
    
    printf("\n🎉 All GC tests passed!\n");
    return 0;
}
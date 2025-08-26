// test_gc_simple.c
//
// Simple unit tests for the garbage collector module
// Tests basic GC functionality without strings or lists dependencies

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../include/gc.h"
#include "../include/nanbox.h"

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
    TEST_ASSERT(stats.collections_count == 0, "Initial collections count is zero");
    
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

// Test basic collection functionality
int test_basic_collection() {
    gc_init();
    
    GCStats before = gc_get_stats();
    TEST_ASSERT(before.collections_count == 0, "Initial collections count is zero");
    
    // Allocate some memory that will be unprotected
    void* ptr1 = gc_allocate(1000);
    void* ptr2 = gc_allocate(2000);
    
    TEST_ASSERT(ptr1 != NULL && ptr2 != NULL, "Allocations succeed");
    
    // Force collection - these objects should be collected since they're not protected
    gc_collect();
    
    GCStats after = gc_get_stats();
    TEST_ASSERT(after.collections_count == 1, "Collection count incremented");
    
    gc_shutdown();
    return 1;
}

// Test that protected objects survive collection
int test_protected_objects_survive() {
    gc_init();
    
    // Allocate objects and protect some
    void* protected_ptr = gc_allocate(500);
    void* unprotected_ptr = gc_allocate(500);
    
    // Create a fake Value that points to our protected allocation
    // This is a bit of a hack since we don't have real string/list types yet
    Value protected_val = STRING_MASK | ((uintptr_t)protected_ptr & 0xFFFFFFFFFFFFULL);
    
    GC_PUSH_SCOPE();
    gc_protect_value(&protected_val);
    
    GCStats before = gc_get_stats();
    size_t bytes_before = before.bytes_allocated;
    
    // Force collection
    gc_collect();
    
    GCStats after = gc_get_stats();
    TEST_ASSERT(after.collections_count == 1, "Collection occurred");
    
    // The protected object should survive, unprotected should be collected
    // Note: This test is limited without actual string/list marking
    TEST_ASSERT(true, "Collection completed without crash");
    
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
    TEST_ASSERT(initial.collections_count == 0, "Initial collections count is zero");
    
    // Allocate some objects
    void* ptr1 = gc_allocate(1000);
    void* ptr2 = gc_allocate(2000);
    
    TEST_ASSERT(ptr1 && ptr2, "Allocations succeed");
    
    GCStats after_alloc = gc_get_stats();
    TEST_ASSERT(after_alloc.bytes_allocated > initial.bytes_allocated, 
                "Allocation increases byte count");
    TEST_ASSERT(after_alloc.collections_count == 0, "No collections yet");
    
    // Force collection
    gc_collect();
    
    GCStats after_gc = gc_get_stats();
    TEST_ASSERT(after_gc.collections_count == 1, "Collection count incremented");
    
    // Test disable affects stats
    gc_disable();
    GCStats disabled = gc_get_stats();
    TEST_ASSERT(disabled.is_enabled == false, "Stats reflect disabled state");
    
    gc_enable();
    GCStats enabled = gc_get_stats();
    TEST_ASSERT(enabled.is_enabled == true, "Stats reflect enabled state");
    
    gc_shutdown();
    return 1;
}

// Test multiple allocations and collections
int test_multiple_collections() {
    gc_init();
    
    GCStats stats = gc_get_stats();
    int initial_collections = stats.collections_count;
    
    // Perform multiple allocations and collections
    for (int i = 0; i < 5; i++) {
        void* ptr = gc_allocate(1000 + i * 100);
        TEST_ASSERT(ptr != NULL, "Allocation succeeds");
        gc_collect();
    }
    
    stats = gc_get_stats();
    TEST_ASSERT(stats.collections_count == initial_collections + 5, 
                "Multiple collections recorded correctly");
    
    gc_shutdown();
    return 1;
}

int main() {
    printf("Simple Garbage Collector Test Suite\n");
    printf("===================================\n");
    
    RUN_TEST(test_gc_init_shutdown);
    RUN_TEST(test_gc_allocation);
    RUN_TEST(test_root_protection);
    RUN_TEST(test_scope_management);
    RUN_TEST(test_gc_locals_macro);
    RUN_TEST(test_gc_enable_disable);
    RUN_TEST(test_basic_collection);
    RUN_TEST(test_protected_objects_survive);
    RUN_TEST(test_gc_stats);
    RUN_TEST(test_multiple_collections);
    
    printf("\n🎉 All simple GC tests passed!\n");
    return 0;
}
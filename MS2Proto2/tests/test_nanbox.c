// test_nanbox.c
//
// Unit tests for the nanbox core module
// Tests NaN-boxing Value type and basic operations in isolation

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
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

// Test nil value creation and checking
int test_nil_operations() {
    Value nil_val = make_nil();
    
    TEST_ASSERT(is_nil(nil_val), "make_nil() creates nil value");
    TEST_ASSERT(!is_int(nil_val), "nil is not int");
    TEST_ASSERT(!is_double(nil_val), "nil is not double");
    TEST_ASSERT(!is_string(nil_val), "nil is not string");
    TEST_ASSERT(!is_list(nil_val), "nil is not list");
    TEST_ASSERT(!is_map(nil_val), "nil is not map");
    TEST_ASSERT(!is_number(nil_val), "nil is not number");
    
    TEST_ASSERT(nil_val == NULL_VALUE, "nil value equals NULL_VALUE constant");
    
    return 1;
}

// Test integer value creation and operations
int test_int_operations() {
    Value int_val = make_int(42);
    Value negative_val = make_int(-123);
    Value zero_val = make_int(0);
    
    // Type checking
    TEST_ASSERT(is_int(int_val), "make_int(42) creates int value");
    TEST_ASSERT(is_int(negative_val), "make_int(-123) creates int value");
    TEST_ASSERT(is_int(zero_val), "make_int(0) creates int value");
    
    TEST_ASSERT(is_number(int_val), "int is number");
    TEST_ASSERT(!is_nil(int_val), "int is not nil");
    TEST_ASSERT(!is_double(int_val), "int is not double");
    TEST_ASSERT(!is_string(int_val), "int is not string");
    TEST_ASSERT(!is_list(int_val), "int is not list");
    TEST_ASSERT(!is_map(int_val), "int is not map");
    
    // Value extraction
    TEST_ASSERT(as_int(int_val) == 42, "as_int(make_int(42)) == 42");
    TEST_ASSERT(as_int(negative_val) == -123, "as_int(make_int(-123)) == -123");
    TEST_ASSERT(as_int(zero_val) == 0, "as_int(make_int(0)) == 0");
    
    // Test some edge cases
    Value max_int = make_int(0x7FFFFFFF);  // Max 32-bit signed int
    Value min_int = make_int((int32_t)0x80000000);  // Min 32-bit signed int
    
    TEST_ASSERT(is_int(max_int), "Max int32 creates valid int value");
    TEST_ASSERT(is_int(min_int), "Min int32 creates valid int value");
    TEST_ASSERT(as_int(max_int) == 0x7FFFFFFF, "Max int32 roundtrip works");
    TEST_ASSERT(as_int(min_int) == (int32_t)0x80000000, "Min int32 roundtrip works");
    
    return 1;
}

// Test double value creation and operations
int test_double_operations() {
    Value double_val = make_double(3.14159);
    Value zero_double = make_double(0.0);
    Value negative_double = make_double(-2.71828);
    
    // Type checking
    TEST_ASSERT(is_double(double_val), "make_double(3.14159) creates double value");
    TEST_ASSERT(is_double(zero_double), "make_double(0.0) creates double value");
    TEST_ASSERT(is_double(negative_double), "make_double(-2.71828) creates double value");
    
    TEST_ASSERT(is_number(double_val), "double is number");
    TEST_ASSERT(!is_nil(double_val), "double is not nil");
    TEST_ASSERT(!is_int(double_val), "double is not int");
    TEST_ASSERT(!is_string(double_val), "double is not string");
    TEST_ASSERT(!is_list(double_val), "double is not list");
    TEST_ASSERT(!is_map(double_val), "double is not map");
    
    // Value extraction (with small tolerance for floating point comparison)
    double extracted = as_double(double_val);
    TEST_ASSERT(fabs(extracted - 3.14159) < 1e-10, "as_double(make_double(3.14159)) roundtrip works");
    
    TEST_ASSERT(as_double(zero_double) == 0.0, "as_double(make_double(0.0)) == 0.0");
    
    double neg_extracted = as_double(negative_double);
    TEST_ASSERT(fabs(neg_extracted - (-2.71828)) < 1e-10, "Negative double roundtrip works");
    
    // Test special double values
    Value inf_val = make_double(INFINITY);
    Value neg_inf_val = make_double(-INFINITY);
    Value nan_val = make_double(NAN);
    
    TEST_ASSERT(is_double(inf_val), "INFINITY creates double value");
    TEST_ASSERT(is_double(neg_inf_val), "-INFINITY creates double value");
    TEST_ASSERT(is_double(nan_val), "NAN creates double value");
    
    TEST_ASSERT(isinf(as_double(inf_val)), "INFINITY roundtrip preserves infinity");
    TEST_ASSERT(isinf(as_double(neg_inf_val)), "-INFINITY roundtrip preserves infinity");
    TEST_ASSERT(isnan(as_double(nan_val)), "NAN roundtrip preserves NaN");
    
    return 1;
}

// Test map type checking (without accessing map operations)
int test_map_type_checking() {
    // We can't create maps without the maps module,
    // but we can test the type checking masks work correctly
    
    // Manually create a map value (this will be done by make_map when implemented)
    Value map_val = MAP_MASK | 0xCAFEBABE;  // Some fake pointer value
    
    TEST_ASSERT(is_map(map_val), "Map mask creates map value");
    TEST_ASSERT(!is_nil(map_val), "Map is not nil");
    TEST_ASSERT(!is_int(map_val), "Map is not int");
    TEST_ASSERT(!is_double(map_val), "Map is not double");
    TEST_ASSERT(!is_string(map_val), "Map is not string");
    TEST_ASSERT(!is_list(map_val), "Map is not list");
    TEST_ASSERT(!is_number(map_val), "Map is not number");
    
    return 1;
}

// Test tiny string type checking (without accessing string operations)
int test_tiny_string_type_checking() {
    // We can't create tiny strings without the strings module, 
    // but we can test the type checking masks work correctly
    
    // Manually create a tiny string value (this is normally done by make_tiny_string)
    Value tiny_val = TINY_STRING_MASK | 0x0205414268;  // "hA" with length 2, in little-endian
    
    TEST_ASSERT(is_tiny_string(tiny_val), "Tiny string mask creates tiny string value");
    TEST_ASSERT(is_string(tiny_val), "Tiny string is considered a string");
    TEST_ASSERT(!is_heap_string(tiny_val), "Tiny string is not heap string");
    TEST_ASSERT(!is_nil(tiny_val), "Tiny string is not nil");
    TEST_ASSERT(!is_int(tiny_val), "Tiny string is not int");
    TEST_ASSERT(!is_double(tiny_val), "Tiny string is not double");
    TEST_ASSERT(!is_list(tiny_val), "Tiny string is not list");
    TEST_ASSERT(!is_map(tiny_val), "Tiny string is not map");
    TEST_ASSERT(!is_number(tiny_val), "Tiny string is not number");
    
    return 1;
}

// Test heap string type checking (without accessing string operations)
int test_heap_string_type_checking() {
    // We can't create heap strings without the GC module,
    // but we can test the type checking masks work correctly
    
    // Manually create a heap string value (this is normally done by make_string)
    Value heap_val = STRING_MASK | 0x123456789AB;  // Some fake pointer value
    
    TEST_ASSERT(is_heap_string(heap_val), "String mask creates heap string value");
    TEST_ASSERT(is_string(heap_val), "Heap string is considered a string");
    TEST_ASSERT(!is_tiny_string(heap_val), "Heap string is not tiny string");
    TEST_ASSERT(!is_nil(heap_val), "Heap string is not nil");
    TEST_ASSERT(!is_int(heap_val), "Heap string is not int");
    TEST_ASSERT(!is_double(heap_val), "Heap string is not double");
    TEST_ASSERT(!is_list(heap_val), "Heap string is not list");
    TEST_ASSERT(!is_map(heap_val), "Heap string is not map");
    TEST_ASSERT(!is_number(heap_val), "Heap string is not number");
    
    return 1;
}

// Test list type checking (without accessing list operations)
int test_list_type_checking() {
    // We can't create lists without the GC module,
    // but we can test the type checking masks work correctly
    
    // Manually create a list value (this is normally done by make_list)
    Value list_val = LIST_MASK | 0xDEADBEEF;  // Some fake pointer value
    
    TEST_ASSERT(is_list(list_val), "List mask creates list value");
    TEST_ASSERT(!is_nil(list_val), "List is not nil");
    TEST_ASSERT(!is_int(list_val), "List is not int");
    TEST_ASSERT(!is_double(list_val), "List is not double");
    TEST_ASSERT(!is_string(list_val), "List is not string");
    TEST_ASSERT(!is_map(list_val), "List is not map");
    TEST_ASSERT(!is_number(list_val), "List is not number");
    
    return 1;
}

// Test value type name debugging function
int test_value_type_names() {
    Value nil_val = make_nil();
    Value int_val = make_int(42);
    Value double_val = make_double(3.14);
    
    TEST_ASSERT(strcmp(value_type_name(nil_val), "nil") == 0, "nil has correct type name");
    TEST_ASSERT(strcmp(value_type_name(int_val), "int") == 0, "int has correct type name");
    TEST_ASSERT(strcmp(value_type_name(double_val), "double") == 0, "double has correct type name");
    
    // Test manually created values
    Value tiny_val = TINY_STRING_MASK | 0x123;
    Value heap_val = STRING_MASK | 0x456;
    Value list_val = LIST_MASK | 0x789;
    Value map_val = MAP_MASK | 0xABC;
    
    TEST_ASSERT(strcmp(value_type_name(tiny_val), "tiny_string") == 0, "tiny string has correct type name");
    TEST_ASSERT(strcmp(value_type_name(heap_val), "heap_string") == 0, "heap string has correct type name");
    TEST_ASSERT(strcmp(value_type_name(list_val), "list") == 0, "list has correct type name");
    TEST_ASSERT(strcmp(value_type_name(map_val), "map") == 0, "map has correct type name");
    
    return 1;
}

// Test that different types have distinct values
int test_type_distinctness() {
    Value nil_val = make_nil();
    Value int_val = make_int(0);
    Value double_val = make_double(0.0);
    
    // All should be different even though they represent "zero" or "null"
    TEST_ASSERT(nil_val != int_val, "nil != int(0)");
    TEST_ASSERT(nil_val != double_val, "nil != double(0.0)");
    TEST_ASSERT(int_val != double_val, "int(0) != double(0.0)");
    
    // Test that different reference types are distinct
    Value tiny_string = TINY_STRING_MASK | 0x0;
    Value heap_string = STRING_MASK | 0x0; 
    Value list_val = LIST_MASK | 0x0;
    Value map_val = MAP_MASK | 0x0;
    
    TEST_ASSERT(tiny_string != heap_string, "tiny string != heap string (even with same payload)");
    TEST_ASSERT(tiny_string != list_val, "tiny string != list");
    TEST_ASSERT(tiny_string != map_val, "tiny string != map");
    TEST_ASSERT(heap_string != list_val, "heap string != list");
    TEST_ASSERT(heap_string != map_val, "heap string != map");
    TEST_ASSERT(list_val != map_val, "list != map");
    
    return 1;
}

int main() {
    printf("NaN-Boxing Core Module Test Suite\n");
    printf("=================================\n");
    
    RUN_TEST(test_nil_operations);
    RUN_TEST(test_int_operations);
    RUN_TEST(test_double_operations);
    RUN_TEST(test_map_type_checking);
    RUN_TEST(test_tiny_string_type_checking);
    RUN_TEST(test_heap_string_type_checking);
    RUN_TEST(test_list_type_checking);
    RUN_TEST(test_value_type_names);
    RUN_TEST(test_type_distinctness);
    
    printf("\n🎉 All NaN-boxing core tests passed!\n");
    return 0;
}
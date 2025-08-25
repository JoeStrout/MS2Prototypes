#include "nanbox.h"
#include "nanbox_gc.h"
#include <stdio.h>

int main() {
    gc_init();
    
    GC_PUSH_SCOPE();
    GC_LOCALS(str1, str2, str3, str4, str5);
    
    printf("String Equality Optimization Test\n");
    printf("=================================\n\n");
    
    // Test 1: Interned strings should be equal via pointer comparison
    str1 = make_string("hello world");
    str2 = make_string("hello world");  // Same string, should be interned
    
    printf("Test 1: Interned string equality\n");
    printf("str1 == str2 (same content): %s\n", string_equals(str1, str2) ? "TRUE" : "FALSE");
    printf("Pointers equal: %s\n\n", (str1 == str2) ? "YES (fast path)" : "NO");
    
    // Test 2: Different strings should not be equal
    str3 = make_string("goodbye world");
    printf("Test 2: Different string content\n");
    printf("str1 == str3 (diff content): %s\n", string_equals(str1, str3) ? "TRUE" : "FALSE");
    printf("Pointers equal: %s\n\n", (str1 == str3) ? "YES" : "NO (different)");
    
    // Test 3: Tiny strings
    str4 = make_string("hi");
    str5 = make_string("hi");  // Same tiny string content
    printf("Test 3: Tiny string equality\n");
    printf("str4 == str5 (same tiny): %s\n", string_equals(str4, str5) ? "TRUE" : "FALSE");
    printf("Values equal: %s\n\n", (str4 == str5) ? "YES (fast path)" : "NO");
    
    // Test 4: Mixed types (heap vs tiny)
    Value tiny = make_string("hi");
    Value heap = make_string("hi but longer");  // This will be heap allocated
    printf("Test 4: Mixed string types\n");
    printf("tiny == heap (diff content): %s\n", string_equals(tiny, heap) ? "TRUE" : "FALSE");
    
    GC_POP_SCOPE();
    
    gc_shutdown();
    return 0;
}
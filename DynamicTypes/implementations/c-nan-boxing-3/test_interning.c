#include "nanbox.h"
#include "nanbox_gc.h"
#include <stdio.h>

int main() {
    gc_init();
    
    GC_PUSH_SCOPE();
    GC_LOCALS(str1, str2, str3, str4);
    
    printf("String Interning Test\n");
    printf("====================\n\n");
    
    // Test that strings under threshold get interned (same pointer)
    str1 = make_string("hello world");  // 11 bytes < 128 threshold
    str2 = make_string("hello world");  // Same string
    
    printf("Test 1: Short strings should be interned\n");
    printf("str1 = make_string(\"hello world\"): %016llx\n", str1);
    printf("str2 = make_string(\"hello world\"): %016llx\n", str2);
    printf("Same pointer? %s\n\n", (str1 == str2) ? "YES (interned)" : "NO");
    
    // Test that different strings get different pointers
    str3 = make_string("goodbye world"); // Different string
    printf("Test 2: Different strings should have different pointers\n");
    printf("str3 = make_string(\"goodbye world\"): %016llx\n", str3);
    printf("str1 != str3? %s\n\n", (str1 != str3) ? "YES" : "NO");
    
    // Test that very short strings (tiny strings) work as expected
    str4 = make_string("hi");  // 2 bytes, should be tiny string
    printf("Test 3: Tiny strings (<=5 chars)\n");
    printf("str4 = make_string(\"hi\"): %016llx\n", str4);
    printf("Is tiny string? %s\n", is_tiny_string(str4) ? "YES" : "NO");
    printf("As C string: \"%s\"\n\n", as_cstring(str4));
    
    // Test hash values for heap strings
    if (is_heap_string(str1)) {
        String* s = as_string(str1);
        printf("Test 4: Hash values for interned strings\n");
        printf("str1 hash: %u (should not be 0)\n", s->hash);
    }
    
    printf("Memory usage after creating strings: %zu bytes\n", gc.bytes_allocated);
    
    GC_POP_SCOPE();
    
    gc_shutdown();
    return 0;
}
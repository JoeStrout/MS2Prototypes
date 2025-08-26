#include <stdio.h>
#include <string.h>
#include "nanbox_gc.h"
#include "gc.c"

// Example function demonstrating proper shadow stack usage
Value string_processing_example(const char* input) {
    GC_PUSH_SCOPE();
    
    // Declare all local Values and protect them
    Value str = make_null();
    Value words = make_null();
    Value result = make_null();
    
    GC_PROTECT(&str);
    GC_PROTECT(&words);
    GC_PROTECT(&result);
    
    // Process the input string
    str = make_string(input);
    
    // Split into words
    Value delimiter = make_string(" ");
    words = string_split(str, delimiter);
    
    // Process each word (convert to uppercase simulation)
    Value processed_list = make_list(list_count(words));
    for (int i = 0; i < list_count(words); i++) {
        Value word = list_get(words, i);
        // In a real implementation, we'd convert to uppercase here
        // For demo, just add "UPPER_" prefix
        Value prefix = make_string("UPPER_");
        Value processed_word = string_concat(prefix, word);
        list_add(processed_list, processed_word);
    }
    
    // Join back together
    Value space = make_string(" ");
    result = make_string("");  // Start with empty string
    
    for (int i = 0; i < list_count(processed_list); i++) {
        if (i > 0) {
            result = string_concat(result, space);
        }
        Value word = list_get(processed_list, i);
        result = string_concat(result, word);
    }
    
    GC_POP_SCOPE();
    
    // Note: result is no longer protected, but that's OK because:
    // 1. We're returning it immediately
    // 2. Collection only happens at safe points
    // 3. The caller should protect it if they need to keep it
    return result;
}

// Another example showing reassignment of protected values
Value fibonacci_strings(int n) {
    GC_PUSH_SCOPE();
    
    Value result_list = make_list(n);
    Value prev = make_string("0");
    Value curr = make_string("1");
    
    GC_PROTECT(&result_list);
    GC_PROTECT(&prev);
    GC_PROTECT(&curr);
    
    if (n > 0) list_add(result_list, prev);
    if (n > 1) list_add(result_list, curr);
    
    for (int i = 2; i < n; i++) {
        // Convert strings to numbers, add, convert back
        // (This is just for demo - in real code we'd use integer Values)
        int prev_val = atoi(as_cstring(prev));
        int curr_val = atoi(as_cstring(curr));
        int next_val = prev_val + curr_val;
        
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d", next_val);
        
        // Reassign protected values - shadow stack handles this cleanly
        prev = curr;
        curr = make_string(buffer);
        
        list_add(result_list, curr);
    }
    
    GC_POP_SCOPE();
    return result_list;
}

int main() {
    printf("Shadow Stack Usage Examples\n");
    printf("===========================\n\n");
    
    gc_init();
    
    // Example 1: String processing
    printf("String processing example:\n");
    Value processed = string_processing_example("hello world test");
    printf("Result: %s\n\n", as_cstring(processed));
    
    // Example 2: Fibonacci with string values
    printf("Fibonacci strings (first 10):\n");
    Value fib_list = fibonacci_strings(10);
    
    for (int i = 0; i < list_count(fib_list); i++) {
        Value item = list_get(fib_list, i);
        printf("fib[%d] = %s\n", i, as_cstring(item));
    }
    
    // Show GC stats
    printf("\nMemory usage: %zu bytes allocated\n", gc.bytes_allocated);
    printf("Performing final collection...\n");
    
    gc_collect();
    printf("After collection: %zu bytes remaining\n", gc.bytes_allocated);
    
    gc_shutdown();
    return 0;
}
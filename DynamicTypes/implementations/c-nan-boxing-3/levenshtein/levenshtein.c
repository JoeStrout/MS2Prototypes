#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "../nanbox_gc.h"

Value editDistance(Value s1_val, Value s2_val) {
    GC_PUSH_SCOPE();
    
    // Declare and protect all local Values
    Value empty_delim = make_null();
    Value s1chars = make_null();
    Value s2chars = make_null();
    Value d_list = make_null();
    
    GC_PROTECT(&empty_delim);
    GC_PROTECT(&s1chars);
    GC_PROTECT(&s2chars);
    GC_PROTECT(&d_list);
    
    if (!is_string(s1_val) || !is_string(s2_val)) {
        GC_POP_SCOPE();
        return make_null();
    }
    
    int n = string_length(s1_val);
    int m = string_length(s2_val);
    
    // Handle edge cases
    if (n == 0) {
        GC_POP_SCOPE();
        return make_int(m);
    }
    if (m == 0) {
        GC_POP_SCOPE();
        return make_int(n);
    }
    
    // Split strings into character lists
    empty_delim = make_string("");
    s1chars = string_split(s1_val, empty_delim);
    s2chars = string_split(s2_val, empty_delim);
    
    // Allocate distance array as a List (more consistent with GC system)
    d_list = make_list(m + 1);
    
    // Initialize d array with range(0, m)
    for (int i = 0; i <= m; i++) {
        list_add(d_list, make_int(i));
    }
    
    int lastCost = 0;
    int nextCost = 0;
    
    // Main algorithm loop  
    for (int i = 1; i <= n; i++) {
        Value s1char = list_get(s1chars, i - 1);
        lastCost = i;
        int jMinus1 = 0;
        
        for (int j = 1; j <= m; j++) {
            Value s2char = list_get(s2chars, jMinus1);
            
            // Calculate cost (0 if characters match, 1 if different)
            int cost = string_equals(s1char, s2char) ? 0 : 1;
            
            // Calculate the three possibilities
            int a = as_int(list_get(d_list, j)) + 1;           // deletion
            int b = lastCost + 1;       // insertion
            int c = cost + as_int(list_get(d_list, jMinus1));  // substitution
            
            // Find minimum using nested if statements (matching original)
            if (a < b) {
                if (c < a) {
                    nextCost = c;
                } else {
                    nextCost = a;
                }
            } else {
                if (c < b) {
                    nextCost = c;
                } else {
                    nextCost = b;
                }
            }
            
            list_set(d_list, jMinus1, make_int(lastCost));
            lastCost = nextCost;
            jMinus1 = j;
        }
        list_set(d_list, m, make_int(lastCost));
    }
    
    int result = nextCost;
    Value final_result = make_int(result);
    
    GC_POP_SCOPE();
    return final_result;
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void runTest() {
    GC_PUSH_SCOPE();
    
    // Declare and protect all local Values
    Value s1 = make_null();
    Value s2 = make_null();
    Value s3 = make_null();
    Value s4 = make_null();
    Value ga1 = make_null();
    Value ga2 = make_null();
    Value banana = make_null();
    Value result1 = make_null();
    Value result2 = make_null();
    Value result3 = make_null();
    Value result4 = make_null();
    
    GC_PROTECT(&s1);
    GC_PROTECT(&s2);
    GC_PROTECT(&s3);
    GC_PROTECT(&s4);
    GC_PROTECT(&ga1);
    GC_PROTECT(&ga2);
    GC_PROTECT(&banana);
    GC_PROTECT(&result1);
    GC_PROTECT(&result2);
    GC_PROTECT(&result3);
    GC_PROTECT(&result4);
    
    // Test 1: "kitten" -> "sitting" = 3
    s1 = make_string("kitten");
    s2 = make_string("sitting");
    result1 = editDistance(s1, s2);
    
    // Test 2: "this is a test..." -> "that was a test..." 
    s3 = make_string("this is a test of a slightly longer string");
    s4 = make_string("that was a test of a slightly longer string");
    result2 = editDistance(s3, s4);
    
    // Test 3: Gettysburg Address variants
    ga1 = make_string(
"Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal.  Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. "
"But, in a larger sense, we can not dedicate--we can not consecrate--we can not hallow--this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. "
"It is rather for us to be here dedicated to the great task remaining before us--that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion--that we here highly resolve that these dead shall not have died in vain--that this nation, under God, shall have a new birth of freedom--and that government of the people, by the people, for the people, shall not perish from the earth.");

	ga2 = make_string(
"Eighty seven years ago our ancestors brought forth in these parts, a new nation, conceived in freedom, and dedicated to the proposition that all people are created equal.  Now we are engaged in a lousy civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are gathered on a famous battlefield of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is super groovy and cool that we should do this. "
"But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or subtract. The world will little note, nor long remember what we say here (ha ha as if), but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. "
"It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation, with its constitutionally guaranteed separation of church and state, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not disappear from Earth.");
    
    result3 = editDistance(ga1, ga2);
    
    // Test 4: Very different strings
    banana = make_string("banana");
    result4 = editDistance(ga1, banana);
    
    // Store results (we'll print them for verification)
    printf("Test results:\n");
    printf("\"kitten\" -> \"sitting\": %d\n", as_int(result1));
    printf("Short sentence test: %d\n", as_int(result2));
    printf("Gettysburg variants: %d\n", as_int(result3));
    printf("GA1 -> \"banana\": %d\n", as_int(result4));
    
    GC_POP_SCOPE();
}

void runBenchmark() {
    printf("Running levenshtein benchmark...\n");
    double t0 = get_time();
    runTest();
    double t1 = get_time();
    double elapsed = t1 - t0;
    printf("levenshtein time: %.3f seconds\n", elapsed);
}

void runCorrectnessTests() {
    printf("Correctness verification:\n");
    
    // Test basic cases
    Value empty1 = make_string("");
    Value empty2 = make_string("");
    Value hello = make_string("hello");
    Value world = make_string("world");
    
    printf("\"\" -> \"\": %d (expected: 0)\n", as_int(editDistance(empty1, empty2)));
    printf("\"hello\" -> \"\": %d (expected: 5)\n", as_int(editDistance(hello, empty1)));
    printf("\"\" -> \"world\": %d (expected: 5)\n", as_int(editDistance(empty1, world)));
    printf("\"hello\" -> \"hello\": %d (expected: 0)\n", as_int(editDistance(hello, hello)));
    
    // Known test case
    Value kitten = make_string("kitten");
    Value sitting = make_string("sitting");
    int result = as_int(editDistance(kitten, sitting));
    printf("\"kitten\" -> \"sitting\": %d (expected: 3)\n", result);
    
    if (result != 3) {
        printf("ERROR: Basic test case failed!\n");
    } else {
        printf("✓ Basic correctness verified\n");
    }
}

int main() {
    printf("NaN Boxing Levenshtein Benchmark (with GC)\n");
    printf("==========================================\n\n");
    
    // Initialize garbage collector
    gc_init();
    
    printf("Before tests: %zu bytes allocated\n", gc.bytes_allocated);
    
    runCorrectnessTests();
    gc_collect();  // Force final collection
    printf("After gc_collect(): %zu bytes remaining\n", gc.bytes_allocated);

    printf("\n");
    
    printf("After correctness tests: %zu bytes allocated\n", gc.bytes_allocated);
    
    runBenchmark();
    
    printf("After benchmark: %zu bytes allocated\n", gc.bytes_allocated);
    gc_collect();  // Force final collection
    printf("After final GC: %zu bytes remaining\n", gc.bytes_allocated);
    
    // Shutdown garbage collector
    gc_shutdown();
    
    return 0;
}
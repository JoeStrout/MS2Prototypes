#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "../nanbox.h"

Value editDistance(Value s1_val, Value s2_val) {
    if (!is_string(s1_val) || !is_string(s2_val)) {
        return make_nil();
    }
    
    int n = string_length(s1_val);
    int m = string_length(s2_val);
    
    // Handle edge cases
    if (n == 0) return make_int(m);
    if (m == 0) return make_int(n);
    
    // Split strings into character lists
    Value empty_delim = make_string("");
    Value s1chars = string_split(s1_val, empty_delim);
    Value s2chars = string_split(s2_val, empty_delim);
    
    // Allocate distance array (d in the original)
    int* d = malloc((m + 1) * sizeof(int));
    
    // Initialize d array with range(0, m)
    for (int i = 0; i <= m; i++) {
        d[i] = i;
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
            int a = d[j] + 1;           // deletion
            int b = lastCost + 1;       // insertion
            int c = cost + d[jMinus1];  // substitution
            
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
            
            d[jMinus1] = lastCost;
            lastCost = nextCost;
            jMinus1 = j;
        }
        d[m] = lastCost;
    }
    
    int result = nextCost;
    free(d);
    return make_int(result);
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void runTest() {
    // Test 1: "kitten" -> "sitting" = 3
    Value s1 = make_string("kitten");
    Value s2 = make_string("sitting");
    Value result1 = editDistance(s1, s2);
    
    // Test 2: "this is a test..." -> "that was a test..." 
    Value s3 = make_string("this is a test of a slightly longer string");
    Value s4 = make_string("that was a test of a slightly longer string");
    Value result2 = editDistance(s3, s4);
    
    // Test 3: Gettysburg Address variants
    Value ga1 = make_string(
"Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal.  Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. "
"But, in a larger sense, we can not dedicate--we can not consecrate--we can not hallow--this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. "
"It is rather for us to be here dedicated to the great task remaining before us--that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion--that we here highly resolve that these dead shall not have died in vain--that this nation, under God, shall have a new birth of freedom--and that government of the people, by the people, for the people, shall not perish from the earth.");

	Value ga2 = make_string(
"Eighty seven years ago our ancestors brought forth in these parts, a new nation, conceived in freedom, and dedicated to the proposition that all people are created equal.  Now we are engaged in a lousy civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are gathered on a famous battlefield of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is super groovy and cool that we should do this. "
"But, in a larger sense, we can not dedicate -- we can not consecrate -- we can not hallow -- this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or subtract. The world will little note, nor long remember what we say here (ha ha as if), but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. "
"It is rather for us to be here dedicated to the great task remaining before us -- that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion -- that we here highly resolve that these dead shall not have died in vain -- that this nation, with its constitutionally guaranteed separation of church and state, shall have a new birth of freedom -- and that government of the people, by the people, for the people, shall not disappear from Earth.");
    
    Value result3 = editDistance(ga1, ga2);
    
    // Test 4: Very different strings
    Value result4 = editDistance(ga1, make_string("banana"));
    
    // Store results (we'll print them for verification)
    printf("Test results:\n");
    printf("\"kitten\" -> \"sitting\": %d\n", as_int(result1));
    printf("Short sentence test: %d\n", as_int(result2));
    printf("Gettysburg variants: %d\n", as_int(result3));
    printf("GA1 -> \"banana\": %d\n", as_int(result4));
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
    printf("NaN Boxing Levenshtein Benchmark\n");
    printf("================================\n\n");
    
    runCorrectnessTests();
    printf("\n");
    
    runBenchmark();
    
    return 0;
}
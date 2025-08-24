#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../nanbox_gc.h"

// Global word arrays - equivalent to MiniScript's split arrays
Value singles;
Value teens;
Value tys;
Value ions;

void initializeWordArrays() {
    // singles = " one two three four five six seven eight nine ".split
    Value singles_str = make_string(" one two three four five six seven eight nine ");
    Value space = make_string(" ");
    GC_PROTECT(singles_str);
    GC_PROTECT(space);
    singles = string_split(singles_str, space);
    GC_PROTECT(singles);
    
    // teens = "ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen ".split
    Value teens_str = make_string("ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen ");
    GC_PROTECT(teens_str);
    teens = string_split(teens_str, space);
    GC_PROTECT(teens);
    
    // tys = "  twenty thirty forty fifty sixty seventy eighty ninety".split
    Value tys_str = make_string("  twenty thirty forty fifty sixty seventy eighty ninety");
    GC_PROTECT(tys_str);
    tys = string_split(tys_str, space);
    GC_PROTECT(tys);
    
    // ions = "thousand million billion".split
    Value ions_str = make_string("thousand million billion");
    GC_PROTECT(ions_str);
    ions = string_split(ions_str, space);
    GC_PROTECT(ions);
    
    // Clean up temporary strings but KEEP the global arrays protected for the entire program
    GC_UNPROTECT(); // ions_str
    GC_UNPROTECT(); // tys_str
    GC_UNPROTECT(); // teens_str
    GC_UNPROTECT(); // space
    GC_UNPROTECT(); // singles_str
    // NOTE: singles, teens, tys, ions remain protected and should not be unprotected
}

Value numberToText(long n) {
    GC_PUSH_SCOPE();
    gc_disable();  // Enter critical section - no GC during complex string operations
    
    if (n == 0) {
        Value result = make_string("zero");
        gc_enable();   // Exit critical section
        GC_POP_SCOPE_AND_RETURN(result);
    }
    
    long a = labs(n);
    Value r = make_string("");  // result
    
    // Process each scale (units, thousands, millions, billions)
    for (int ionIndex = 0; ionIndex < list_count(ions); ionIndex++) {
        Value u = list_get(ions, ionIndex);
        
        long h = a % 100;
        if (h > 0 && h < 10) {
            Value singles_item = list_get(singles, h);
            Value space_val = make_string(" ");
            Value temp = string_concat(singles_item, space_val);
            r = string_concat(temp, r);
        }
        if (h > 9 && h < 20) {
            Value teens_item = list_get(teens, h - 10);
            Value space_val = make_string(" ");
            Value temp = string_concat(teens_item, space_val);
            r = string_concat(temp, r);
        }
        if (h > 19 && h < 100) {
            Value tys_item = list_get(tys, h / 10);
            Value hyphen = (h % 10 > 0) ? make_string("-") : make_string("");
            Value singles_item = list_get(singles, h % 10);
            Value space_val = make_string(" ");
            
            Value temp1 = string_concat(tys_item, hyphen);
            Value temp2 = string_concat(temp1, singles_item);
            Value temp3 = string_concat(temp2, space_val);
            r = string_concat(temp3, r);
        }
        
        h = (a % 1000) / 100;
        if (h) {
            Value singles_item = list_get(singles, h);
            Value hundred_space = make_string(" hundred ");
            Value temp = string_concat(singles_item, hundred_space);
            r = string_concat(temp, r);
        }
        
        a = a / 1000;
        if (a == 0) break;
        if (a % 1000 > 0) {
            Value space_val = make_string(" ");
            Value temp = string_concat(u, space_val);
            r = string_concat(temp, r);
        }
    }
    
    if (n < 0) {
        Value negative = make_string("negative ");
        r = string_concat(negative, r);
    }
    
    // Trim the result (remove leading/trailing spaces)
    const char* r_str = as_cstring(r);
    int len = string_length(r);
    
    // Find first non-space
    int start = 0;
    while (start < len && r_str[start] == ' ') start++;
    
    // Find last non-space
    int end = len - 1;
    while (end >= 0 && r_str[end] == ' ') end--;
    
    Value result;
    if (start <= end) {
        char* trimmed = malloc(end - start + 2);
        strncpy(trimmed, r_str + start, end - start + 1);
        trimmed[end - start + 1] = '\0';
        result = make_string(trimmed);
        free(trimmed);
    } else {
        result = make_string("");
    }
    gc_enable();   // Exit critical section
    GC_POP_SCOPE_AND_RETURN(result);
}

long textToNumber(Value s) {
    gc_disable();  // Enter critical section - no GC during complex string operations
    
    if (!is_string(s)) {
        gc_enable();   // Exit critical section
        return 0;
    }
    
    Value zero_str = make_string("zero");
    if (string_equals(s, zero_str)) {
        gc_enable();   // Exit critical section
        return 0;
    }
    
    // Replace hyphens with spaces and split
    Value hyphen = make_string("-");
    Value space_val = make_string(" ");
    Value clean_s = string_replace(s, hyphen, space_val);
    Value words = string_split(clean_s, space_val);
    
    long result = 0;
    long ionVal = 0;
    int negative = 0;
    int maxi = list_count(words) - 1;
    int i = 0;
    
    while (i <= maxi) {
        Value word = list_get(words, i);
        
        Value negative_str = make_string("negative");
        if (string_equals(word, negative_str)) {
            negative = 1;
            i++;
            continue;
        }
        
        // Check for scale words (thousand, million, billion)
        int idx = list_indexOf(ions, word);
        if (idx != -1) {
            long multipliers[] = {1000, 1000000, 1000000000};
            result += ionVal * multipliers[idx];
            ionVal = 0;
            i++;
            continue;
        }
        
        long wordVal = 0;
        
        // Check singles
        idx = list_indexOf(singles, word);
        if (idx != -1) {
            wordVal = idx;
        } else {
            // Check tys (twenties, thirties, etc.)
            idx = list_indexOf(tys, word);
            if (idx != -1) {
                wordVal = idx * 10;
            } else {
                // Check teens
                idx = list_indexOf(teens, word);
                if (idx != -1) {
                    wordVal = idx + 10;
                } else {
                    printf("Unexpected word: %s\n", as_cstring(word));
                    return 0;
                }
            }
        }
        
        // Handle "hundred"
        if (i < maxi) {
            Value next_word = list_get(words, i + 1);
            Value hundred_str = make_string("hundred");
            if (string_equals(next_word, hundred_str)) {
                wordVal *= 100;
                i++;
            }
        }
        
        ionVal += wordVal;
        i++;
    }
    
    result += ionVal;
    if (negative) result = -result;
    
    gc_enable();   // Exit critical section
    return result;
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void runBenchmark(long n) {
    double t0 = get_time();
    
    for (long i = 0; i < n; i++) {
        Value s = numberToText(i);
        // s is already protected by numberToText scope system
        long i2 = textToNumber(s);
        if (i2 != i) {
            printf("Oops! Failed on %ld:\n", i);
            printf("'%s' --> %ld\n", as_cstring(s), i2);
        }
        GC_UNPROTECT(); // s from numberToText
    }
    
    double t1 = get_time();
    double elapsed = t1 - t0;
    
    printf("numberWords(%ld) time: %.3f seconds\n", n, elapsed);
}

void runCorrectnessTests() {
    printf("Correctness checks:\n");
    
    long testNumbers[] = {-1234, 0, 7, 42, 4325, 1000004, 214837564};
    int numTests = sizeof(testNumbers) / sizeof(testNumbers[0]);
    
    for (int i = 0; i < numTests; i++) {
        long n = testNumbers[i];
        Value words = numberToText(n);
        // words is already protected by numberToText scope system
        long backToNum = textToNumber(words);
        
        printf("%ld: %s -> %ld", n, as_cstring(words), backToNum);
        if (backToNum != n) {
            printf(" ERROR --^");
            GC_UNPROTECT(); // words from numberToText
            return;
        }
        printf("\n");
        GC_UNPROTECT(); // words from numberToText
    }
}

int main() {
    printf("NaN Boxing NumberWords Benchmark (with GC)\n");
    printf("==========================================\n\n");
    
    // Initialize garbage collector
    gc_init();
    
    printf("Before initialization: %zu bytes allocated\n", gc.bytes_allocated);
    initializeWordArrays();
    printf("After initialization: %zu bytes allocated\n", gc.bytes_allocated);
    
    runCorrectnessTests();
    printf("After correctness tests: %zu bytes allocated\n", gc.bytes_allocated);
    printf("\n");
    
    runBenchmark(10000);
    
    printf("After benchmark: %zu bytes allocated\n", gc.bytes_allocated);
    gc_collect();  // Force final collection
    printf("After final GC: %zu bytes remaining\n", gc.bytes_allocated);
    
    // Shutdown garbage collector
    gc_shutdown();
    
    return 0;
}
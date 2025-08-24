#include <stdio.h>
#include <time.h>
#include "../nanbox_gc.h"

Value rfib(Value n_val) {
    // Protect local variables from GC
    GC_PROTECT(n_val);
    Value result = make_nil();
    GC_PROTECT(result);
    
    if (!is_int(n_val)) {
        result = make_nil();
        goto cleanup;
    }
    
    int32_t n = as_int(n_val);
    
    if (n <= 0) {
        result = make_int(0);
        goto cleanup;
    }
    if (n <= 2) {
        result = make_int(1);
        goto cleanup;
    }
    
    Value n_minus_1 = make_int(n - 1);
    Value n_minus_2 = make_int(n - 2);
    GC_PROTECT(n_minus_1);
    GC_PROTECT(n_minus_2);
    
    Value fib1 = rfib(n_minus_1);
    Value fib2 = rfib(n_minus_2);
    GC_PROTECT(fib1);
    GC_PROTECT(fib2);
    
    if (!is_int(fib1) || !is_int(fib2)) {
        printf("ERROR: Non-integer result in rfib(%d): fib1=%llx is_int=%d, fib2=%llx is_int=%d\n", 
               n, fib1, is_int(fib1), fib2, is_int(fib2));
        result = make_nil();
        GC_UNPROTECT(); // fib2
        GC_UNPROTECT(); // fib1
        GC_UNPROTECT(); // n_minus_2
        GC_UNPROTECT(); // n_minus_1
        goto cleanup;
    }
    
    int32_t val1 = as_int(fib1);
    int32_t val2 = as_int(fib2);
    int32_t result_val = val1 + val2;
    result = make_int(result_val);
    
    // Clean up protected values
    GC_UNPROTECT(); // fib2
    GC_UNPROTECT(); // fib1
    GC_UNPROTECT(); // n_minus_2
    GC_UNPROTECT(); // n_minus_1

cleanup:
    GC_UNPROTECT(); // result
    GC_UNPROTECT(); // n_val
    return result;
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void run_benchmark(int n) {
    Value n_val = make_int(n);
    Value result = make_nil();
    GC_PROTECT(n_val);
    GC_PROTECT(result);
    
    printf("Testing with n=%d, n_val=0x%llx, as_int=%d\n", 
           n, n_val, as_int(n_val));
    
    double t0 = get_time();
    result = rfib(n_val);
    double t1 = get_time();
    
    printf("rfib(%d) = %d, time: %.3f seconds\n", 
           n, as_int(result), t1 - t0);
           
    GC_UNPROTECT(); // result
    GC_UNPROTECT(); // n_val
}

int main() {
    printf("NaN Boxing Fibonacci Benchmark (with GC)\n");
    printf("========================================\n");
    
    // Initialize garbage collector
    gc_init();
    
    printf("Testing small cases:\n");
    for (int i = 0; i <= 5; i++) {
        Value n_val = make_int(i);
        Value result = make_nil();
        GC_PROTECT(n_val);
        GC_PROTECT(result);
        
        result = rfib(n_val);
        printf("rfib(%d) = %d\n", i, as_int(result));
        
        GC_UNPROTECT(); // result
        GC_UNPROTECT(); // n_val
    }
    
    printf("\nBenchmark results:\n");
    run_benchmark(30);
    
    printf("\nFinal GC stats: %zu bytes allocated\n", gc.bytes_allocated);
    gc_collect();  // Force final collection
    printf("After GC: %zu bytes remaining\n", gc.bytes_allocated);
    
    // Shutdown garbage collector
    gc_shutdown();
    
    return 0;
}
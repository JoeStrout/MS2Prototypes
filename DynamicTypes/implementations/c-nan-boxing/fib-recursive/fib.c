#include <stdio.h>
#include <time.h>
#include "nanbox.h"

Value rfib(Value n_val) {
    if (!is_int(n_val)) {
        return make_nil();
    }
    
    int32_t n = as_int(n_val);
    
    if (n <= 0) return make_int(0);
    if (n <= 2) return make_int(1);
    
    Value n_minus_1 = make_int(n - 1);
    Value n_minus_2 = make_int(n - 2);
    
    Value fib1 = rfib(n_minus_1);
    Value fib2 = rfib(n_minus_2);
    
    if (!is_int(fib1) || !is_int(fib2)) {
        printf("ERROR: Non-integer result in rfib(%d): fib1=%llx is_int=%d, fib2=%llx is_int=%d\n", 
               n, fib1, is_int(fib1), fib2, is_int(fib2));
        return make_nil();
    }
    
    int32_t val1 = as_int(fib1);
    int32_t val2 = as_int(fib2);
    int32_t result = val1 + val2;
    //printf("rfib(%d): %d + %d = %d\n", n, val1, val2, result);
    return make_int(result);
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void run_benchmark(int n) {
    Value n_val = make_int(n);
    printf("Testing with n=%d, n_val=0x%llx, as_int=%d\n", 
           n, n_val, as_int(n_val));
    
    double t0 = get_time();
    Value result = rfib(n_val);
    double t1 = get_time();
    
    printf("rfib(%d) = %d, time: %.3f seconds\n", 
           n, as_int(result), t1 - t0);
}

int main() {
    printf("NaN Boxing Fibonacci Benchmark\n");
    printf("==============================\n");
    
    printf("Testing small cases:\n");
    for (int i = 0; i <= 5; i++) {
        Value result = rfib(make_int(i));
        printf("rfib(%d) = %d\n", i, as_int(result));
    }
    
     printf("\nBenchmark results:\n");
     run_benchmark(30);
    
    return 0;
}
#include <iostream>
#include <chrono>
#include "MiniscriptTypes.h"

using namespace MiniScript;

Value rfib(Value n_val) {
    // Check if the value is a number (integer)
    if (n_val.type != ValueType::Number) {
        return Value::null;
    }
    
    int32_t n = n_val.IntValue();
    
    if (n <= 0) return Value(0.0);
    if (n <= 2) return Value(1.0);
    
    Value n_minus_1(static_cast<double>(n - 1));
    Value n_minus_2(static_cast<double>(n - 2));
    
    Value fib1 = rfib(n_minus_1);
    Value fib2 = rfib(n_minus_2);
    
    if (fib1.type != ValueType::Number || fib2.type != ValueType::Number) {
        std::cout << "ERROR: Non-number result in rfib(" << n << ")" << std::endl;
        return Value::null;
    }
    
    int32_t val1 = fib1.IntValue();
    int32_t val2 = fib2.IntValue();
    int32_t result = val1 + val2;
    
    //std::cout << "rfib(" << n << "): " << val1 << " + " << val2 << " = " << result << std::endl;
    
    return Value(static_cast<double>(result));
}

double get_time() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    return nanoseconds.count() / 1000000000.0;
}

void run_benchmark(int n) {
    Value n_val(static_cast<double>(n));
    std::cout << "Testing with n=" << n << ", n_val=" << n_val.IntValue() << std::endl;
    
    double t0 = get_time();
    Value result = rfib(n_val);
    double t1 = get_time();
    
    std::cout << "rfib(" << n << ") = " << result.IntValue() 
              << ", time: " << (t1 - t0) << " seconds" << std::endl;
}

int main() {
    std::cout << "MiniScript::Value Fibonacci Benchmark" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    std::cout << "Testing small cases:" << std::endl;
    for (int i = 0; i <= 5; i++) {
        Value result = rfib(Value(static_cast<double>(i)));
        std::cout << "rfib(" << i << ") = " << result.IntValue() << std::endl;
    }
    
    std::cout << "\nBenchmark results:" << std::endl;
    run_benchmark(30);
    
    return 0;
}
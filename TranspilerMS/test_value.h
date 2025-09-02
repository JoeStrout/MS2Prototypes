#pragma once

// Minimal Value type for testing VM compilation
struct Value {
    int type;
    union {
        int intValue;
        double doubleValue;
    };
    
    Value() : type(0), intValue(0) {}
    Value(int i) : type(1), intValue(i) {}
    Value(double d) : type(2), doubleValue(d) {}
};
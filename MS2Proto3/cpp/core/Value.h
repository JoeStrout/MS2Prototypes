#pragma once
#include <cstdint>

// Forward declaration for GC system
class GC;

// NaN-boxed Value type for MiniScript runtime
// This is a placeholder - full implementation will use NaN boxing
struct Value {
    enum Type {
        NIL = 0,
        BOOL = 1,
        INT = 2,
        FLOAT = 3,
        STRING = 4,
        LIST = 5,
        MAP = 6
    };
    
    Type type;
    union {
        bool boolValue;
        int32_t intValue;
        double floatValue;
        void* ptrValue; // For GC-managed objects (strings, lists, maps)
    };
    
    // Constructors
    Value() : type(NIL), intValue(0) {}
    Value(bool b) : type(BOOL), boolValue(b) {}
    Value(int32_t i) : type(INT), intValue(i) {}
    Value(double f) : type(FLOAT), floatValue(f) {}
    
    // Static factory methods
    static Value Null() { return Value(); }
    static Value FromInt(int32_t i) { return Value(i); }
    static Value FromFloat(double f) { return Value(f); }
    static Value FromBool(bool b) { return Value(b); }
    
    // Operations
    static Value Add(const Value& a, const Value& b);
    static Value Sub(const Value& a, const Value& b);
    static bool LessThan(const Value& a, const Value& b);
    
    // Type checking
    bool IsNull() const { return type == NIL; }
    bool IsBool() const { return type == BOOL; }
    bool IsInt() const { return type == INT; }
    bool IsFloat() const { return type == FLOAT; }
    bool IsString() const { return type == STRING; }
    
    // Copy constructor and assignment operator (trivial - just copy 8 bytes)
    Value(const Value& other) = default;
    Value& operator=(const Value& other) = default;
};
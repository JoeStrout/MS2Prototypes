#include "Value.h"
#include <cstring>

Value& Value::operator=(const Value& other) {
    type = other.type;
    switch (type) {
        case NIL: break;
        case BOOL: boolValue = other.boolValue; break;
        case INT: intValue = other.intValue; break;
        case FLOAT: floatValue = other.floatValue; break;
        case STRING:
        case LIST:
        case MAP: ptrValue = other.ptrValue; break;
    }
    return *this;
}

Value Value::Add(const Value& a, const Value& b) {
    if (a.type == INT && b.type == INT) {
        return Value(a.intValue + b.intValue);
    }
    if ((a.type == INT || a.type == FLOAT) && (b.type == INT || b.type == FLOAT)) {
        double aVal = (a.type == INT) ? a.intValue : a.floatValue;
        double bVal = (b.type == INT) ? b.intValue : b.floatValue;
        return Value(aVal + bVal);
    }
    // TODO: String concatenation, other types
    return Value::Null();
}

Value Value::Sub(const Value& a, const Value& b) {
    if (a.type == INT && b.type == INT) {
        return Value(a.intValue - b.intValue);
    }
    if ((a.type == INT || a.type == FLOAT) && (b.type == INT || b.type == FLOAT)) {
        double aVal = (a.type == INT) ? a.intValue : a.floatValue;
        double bVal = (b.type == INT) ? b.intValue : b.floatValue;
        return Value(aVal - bVal);
    }
    return Value::Null();
}

bool Value::LessThan(const Value& a, const Value& b) {
    if (a.type == INT && b.type == INT) {
        return a.intValue < b.intValue;
    }
    if ((a.type == INT || a.type == FLOAT) && (b.type == INT || b.type == FLOAT)) {
        double aVal = (a.type == INT) ? a.intValue : a.floatValue;
        double bVal = (b.type == INT) ? b.intValue : b.floatValue;
        return aVal < bVal;
    }
    // TODO: String comparison, other types
    return false;
}
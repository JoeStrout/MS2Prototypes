// generated — DO NOT EDIT
#pragma once
#include "mini_runtime.hpp"

// Bytecode: a test class for testing the C# --> C++ transpiler.
class Bytecode {
public:
    int meaning = 42;
private:
    int secret = 007;
private:
    Mini.List<Value> __prop_Consts;
public:
    int AddConst(Value v);
    inline Mini.List<Value> get_Consts() const { return __prop_Consts; }
};


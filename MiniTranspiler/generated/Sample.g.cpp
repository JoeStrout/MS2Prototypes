// generated — DO NOT EDIT
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
int Bytecode::AddConst(Value v) {
  // body
  {
        // Deduplication: bail out if constant already exists
        for (int __i0=0, __n1=__prop_Consts.Count(); __i0<__n1; ++__i0) {
          Value c = __prop_Consts[__i0];
          if (c.Equals(v)) return -1;  // (a silly result, but we're just testing here)
        }
        __prop_Consts.Add(v);
        return __prop_Consts.Count();
      }
}
inline Mini.List<Value> get_Consts() const { return __prop_Consts; }
};


#include "Mini.h"

namespace "MiniDemo" {

  // Bytecode: a test class for testing the C# --> C++ transpiler.
  class Bytecode {
    public: int meaning = 42;
    public: Mini.List<Value> Consts = new Mini.List<Value>();
    private: int secret = 007;  // Bond.  James Bond.

    public: int AddConst(Value v);

    public: int CountPlusOne => Consts.Count + 1; // expression-bodied property (will be lowered)
  }
}

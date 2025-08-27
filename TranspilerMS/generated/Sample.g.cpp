#include "Mini.h"

namespace "MiniDemo" {

  // Bytecode: a test class for testing the C# --> C++ transpiler.
  class Bytecode {

    int AddConst(Value v) {
      // Deduplication: bail out if constant already exists
      for (int __c_idx=0, __c_qty=Consts.Count(); __c_idx < __c_qty; __c_idx++) {
      	var c = Consts[__c_idx];
        if (c.Equals(v)) return -1;  // (a silly result, but we're just testing here)
      }
      Consts.Add(v);
      return Consts.Count;
    }

  }
}

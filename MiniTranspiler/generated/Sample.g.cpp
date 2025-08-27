// generated — DO NOT EDIT
#include "Sample.g.h"

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

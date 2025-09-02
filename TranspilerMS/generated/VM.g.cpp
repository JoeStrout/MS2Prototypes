#include "VM.g.h"

namespace ScriptingVM {
    // Opcodes matching the C VM implementation

    // Instruction field extraction helpers (matching C implementation)

        // Instruction encoding helpers

    // Function prototype (equivalent to C Proto struct)


        Proto::Proto(List<UInt32> code, UInt16 maxRegs, List<Value> constants) {
            Code = code;
            MaxRegs = maxRegs;
            Constants = constants;
            if (!Code) Code = List<UInt32>();
            if (!Constants) Constants = List<Value>();
        }

        Proto::Proto() {
            Code = List<UInt32>();
        	MaxRegs = 0; // frame reservation size
        	Constants = List<Value>();
        }

}

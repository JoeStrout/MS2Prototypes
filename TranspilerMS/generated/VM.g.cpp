#include "VM.g.h"

namespace "ScriptingVM" {
    // Opcodes matching the C VM implementation
        MOVE = 0,
        LOADK = 1,
        LOADN = 2,
        ADD = 3,
        SUB = 4,
        IFLT = 5,
        JMP = 6,
        CALLF = 7,
        RETURN = 8

    // Instruction field extraction helpers (matching C implementation)

        // Instruction encoding helpers

    // Function prototype (equivalent to C Proto struct)
        public List<uint> Code = new List<uint>();
        public List<Value> Constants = new List<Value>();


        Proto::Proto(List<uint> code, ushort maxRegs, List<Value> constants) {
            Code = code;
            MaxRegs = maxRegs;
            Constants = constants;
            if (!Code) Code = new List<uint>();
            if (!Constants) Constants = new List<Value>();
        }

        Proto::Proto() {
            Code = new List<uint>();
        	MaxRegs = 0; // frame reservation size
        	Constants = new List<Value>();
        }


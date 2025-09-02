#include "core_includes.h"

namespace ScriptingVM {
    // Opcodes matching the C VM implementation
    enum class Opcode : Byte {
        MOVE = 0,
        LOADK = 1,
        LOADN = 2,
        ADD = 3,
        SUB = 4,
        IFLT = 5,
        JMP = 6,
        CALLF = 7,
        RETURN = 8
    }; // end of class Opcode

    // Instruction field extraction helpers (matching C implementation)
    class InstructionHelpers {
        public: static Byte OP(UInt32 instruction) { return (Byte)((instruction >> 24) & 0xFF); }
        public: static Byte A(UInt32 instruction) { return (Byte)((instruction >> 16) & 0xFF); }
        public: static Byte B(UInt32 instruction) { return (Byte)((instruction >> 8) & 0xFF); }
        public: static Byte C(UInt32 instruction) { return (Byte)(instruction & 0xFF); }
        public: static Int16 BC(UInt32 instruction) { return (Int16)(instruction & 0xFFFF); }

        // Instruction encoding helpers
        public: static UInt32 INS(Opcode op) { return (UInt32)((Byte)op << 24); }
        public: static UInt32 INS_ABC(Opcode op, Byte a, Byte b, Byte c) { return (UInt32)(((Byte)op << 24) | (a << 16) | (b << 8) | c); }
        public: static UInt32 INS_AB(Opcode op, Byte a, Int16 bc) { return (UInt32)(((Byte)op << 24) | (a << 16) | ((UInt16)bc)); }
    }; // end of class InstructionHelpers

    // Function prototype (equivalent to C Proto struct)
    class Proto {
        public: List<UInt32> Code = List<UInt32>();
        public: UInt16 MaxRegs; // frame reservation size
        public: List<Value> Constants = List<Value>();


        public: Proto(List<UInt32> code, UInt16 maxRegs, List<Value> constants);

        public: Proto();
    }; // end of class Proto

}

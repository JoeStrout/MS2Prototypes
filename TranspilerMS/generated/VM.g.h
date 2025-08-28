#include "core_includes.h"

namespace "ScriptingVM" {
    // Opcodes matching the C VM implementation
    enum class Opcode : byte {
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
        public: static byte OP(uint instruction) { return (byte)((instruction >> 24) & 0xFF); }
        public: static byte A(uint instruction) { return (byte)((instruction >> 16) & 0xFF); }
        public: static byte B(uint instruction) { return (byte)((instruction >> 8) & 0xFF); }
        public: static byte C(uint instruction) { return (byte)(instruction & 0xFF); }
        public: static short BC(uint instruction) { return (short)(instruction & 0xFFFF); }

        // Instruction encoding helpers
        public: static uint INS(Opcode op) { return (uint)((byte)op << 24); }
        public: static uint INS_ABC(Opcode op, byte a, byte b, byte c) { return (uint)(((byte)op << 24) | (a << 16) | (b << 8) | c); }
        public: static uint INS_AB(Opcode op, byte a, short bc) { return (uint)(((byte)op << 24) | (a << 16) | ((ushort)bc)); }
    }; // end of class InstructionHelpers

    // Function prototype (equivalent to C Proto struct)
    class Proto {
        public List<uint> Code = new List<uint>();
        public: ushort MaxRegs; // frame reservation size
        public List<Value> Constants = new List<Value>();

        public: int CodeLength() { return Code.Count; }
        public: int ConstLength() { return Constants.Count; }

        public: Proto(List<uint> code, ushort maxRegs, List<Value> constants);

        public: Proto();
    }; // end of class Proto


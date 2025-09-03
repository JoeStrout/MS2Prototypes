using System;
using System.Collections.Generic;

namespace MiniScript {
    // Opcodes
    public enum Opcode : Byte {
    	NOOP = 0,
    	// 1-9: Simple register loads
        MOVE = 1,
        LOADK = 2,
        LOADN = 3,
        // 10s: Math operations
        ADD = 10,
        SUB = 11,
        // 20s: jumps/branches
        JMP = 20,
        IFLT = 21,
        // 30s: function support
        CALLF = 30,
        RETURN = 31
    }

    public static class BytecodeUtil {
	    // Instruction field extraction helpers
        public static Byte OP(UInt32 instruction) => (Byte)((instruction >> 24) & 0xFF);
        public static Byte A(UInt32 instruction) => (Byte)((instruction >> 16) & 0xFF);
        public static Byte B(UInt32 instruction) => (Byte)((instruction >> 8) & 0xFF);
        public static Byte C(UInt32 instruction) => (Byte)(instruction & 0xFF);
        public static Int16 BC(UInt32 instruction) => (Int16)(instruction & 0xFFFF);

        // Instruction encoding helpers
        public static UInt32 INS(Opcode op) => (UInt32)((Byte)op << 24);
        public static UInt32 INS_AB(Opcode op, Byte a, Int16 bc) => (UInt32)(((Byte)op << 24) | (a << 16) | ((UInt16)bc));
        public static UInt32 INS_ABC(Opcode op, Byte a, Byte b, Byte c) => (UInt32)(((Byte)op << 24) | (a << 16) | (b << 8) | c);
        
        // Conversion to/from opcode mnemonics (names)
        public static String ToMnemonic(Opcode opcode) {
        	switch (opcode) {
        		case Opcode.NOOP:   return "NOOP";
        		case Opcode.MOVE:   return "MOVE";
        		case Opcode.LOADK:  return "LOADK";
        		case Opcode.LOADN:  return "LOADN";
        		case Opcode.ADD:    return "ADD";
        		case Opcode.SUB:    return "SUB";
        		case Opcode.JMP:    return "JMP";
        		case Opcode.IFLT:   return "IFLT";
				case Opcode.CALLF:  return "CALLF";
        		case Opcode.RETURN: return "RETURN";
        		default:
        			return "Unknown opcode";
        	}
        }
        
        public static Opcode FromMnemonic(String s) {
        	if (s == "NOOP")   return Opcode.NOOP;
			if (s == "MOVE")   return Opcode.MOVE;
			if (s == "LOADK")  return Opcode.LOADK;
			if (s == "LOADN")  return Opcode.LOADN;
			if (s == "ADD")    return Opcode.ADD;
			if (s == "SUB")    return Opcode.SUB;
			if (s == "JMP")    return Opcode.JMP;
			if (s == "IFLT")   return Opcode.IFLT;
			if (s == "CALLF")  return Opcode.CALLF;
			if (s == "RETURN") return Opcode.RETURN;
        	return Opcode.NOOP;
        }
    }
}


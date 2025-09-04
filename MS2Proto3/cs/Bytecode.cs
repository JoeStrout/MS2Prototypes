using System;
using System.Collections.Generic;

namespace MiniScript {
	// Opcodes.  Note that these must have sequential values, starting at 0.
	public enum Opcode : Byte {
		NOOP = 0,
		LOAD_rA_rB = 1,
		LOAD_rA_iBC = 2,
		LOAD_rA_kBC = 3,
		ADD_rA_rB_rC = 4,
		SUB_rA_rB_rC = 5,
		JUMP_iABC = 6,
		IFLT_rA_rB = 7,
		IFLT_rA_iBC = 8,
		CALLF_iA_iBC = 9,
		RETURN = 10
	}

	public static class BytecodeUtil {
		// Instruction field extraction helpers
		public static Byte OP(UInt32 instruction) => (Byte)((instruction >> 24) & 0xFF);
		public static Byte A(UInt32 instruction) => (Byte)((instruction >> 16) & 0xFF);
		public static Byte B(UInt32 instruction) => (Byte)((instruction >> 8) & 0xFF);
		public static Byte C(UInt32 instruction) => (Byte)(instruction & 0xFF);
		public static Int16 BC(UInt32 instruction) => (Int16)(instruction & 0xFFFF);
		public static UInt16 BCu(UInt32 instruction) => (UInt16)(instruction & 0xFFFF);
		public static Int32 ABC(UInt32 instruction) => (Int32)(instruction & 0xFFFFFF);

		// Instruction encoding helpers
		public static UInt32 INS(Opcode op) => (UInt32)((Byte)op << 24);
		public static UInt32 INS_AB(Opcode op, Byte a, Int16 bc) => (UInt32)(((Byte)op << 24) | (a << 16) | ((UInt16)bc));
		public static UInt32 INS_ABC(Opcode op, Byte a, Byte b, Byte c) => (UInt32)(((Byte)op << 24) | (a << 16) | (b << 8) | c);
		
		// Conversion to/from opcode mnemonics (names)
		public static String ToMnemonic(Opcode opcode) {
			switch (opcode) {
				case Opcode.NOOP:         return "NOOP";
				case Opcode.LOAD_rA_rB:   return "LOAD_rA_rB";
				case Opcode.LOAD_rA_iBC:  return "LOAD_rA_iBC";
				case Opcode.LOAD_rA_kBC:  return "LOAD_rA_kBC";
				case Opcode.ADD_rA_rB_rC: return "ADD_rA_rB_rC";
				case Opcode.SUB_rA_rB_rC: return "SUB_rA_rB_rC";
				case Opcode.JUMP_iABC:    return "JUMP_iABC";
				case Opcode.IFLT_rA_rB:   return "IFLT_rA_rB";
				case Opcode.IFLT_rA_iBC:  return "IFLT_rA_iBC";
				case Opcode.CALLF_iA_iBC: return "CALLF_iA_iBC";
				case Opcode.RETURN:       return "RETURN";
				default:
					return "Unknown opcode";
			}
		}
		
		public static Opcode FromMnemonic(String s) {
			if (s == "NOOP")         return Opcode.NOOP;
			if (s == "LOAD_rA_rB")   return Opcode.LOAD_rA_rB;
			if (s == "LOAD_rA_iBC")  return Opcode.LOAD_rA_iBC;
			if (s == "LOAD_rA_kBC")  return Opcode.LOAD_rA_kBC;
			if (s == "ADD_rA_rB_rC") return Opcode.ADD_rA_rB_rC;
			if (s == "SUB_rA_rB_rC") return Opcode.SUB_rA_rB_rC;
			if (s == "JUMP_iABC")    return Opcode.JUMP_iABC;
			if (s == "IFLT_rA_rB")   return Opcode.IFLT_rA_rB;
			if (s == "IFLT_rA_iBC")  return Opcode.IFLT_rA_iBC;
			if (s == "CALLF_iA_iBC") return Opcode.CALLF_iA_iBC;
			if (s == "RETURN")       return Opcode.RETURN;
			return Opcode.NOOP;
		}
	}
	
	// Function definition: code, constants, and how many registers it needs
	public class FuncDef {
		public List<UInt32> Code = new List<UInt32>();
		public List<Int32> Constants = new List<Int32>(); // ToDo: use Value instead
		public UInt16 MaxRegs = 0; // frame reservation size
	}

}


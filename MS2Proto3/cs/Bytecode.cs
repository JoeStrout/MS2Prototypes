using System;
using System.Collections.Generic;
// CPP: #include "value.h"

namespace MiniScript {
	// Opcodes.  Note that these must have sequential values, starting at 0.
	public enum Opcode : Byte {
		NOOP = 0,
		LOAD_rA_rB = 1,
		LOAD_rA_iBC = 2,
		LOAD_rA_kBC = 3,
		ADD_rA_rB_rC = 4,
		SUB_rA_rB_rC = 5,
		MULT_rA_rB_rC = 6,
		DIV_rA_rB_rC = 7,
		MOD_rA_rB_rC = 8,
		JUMP_iABC = 9,
		IFLT_rA_rB = 10,
		IFLT_rA_iBC = 11,
		CALLF_iA_iBC = 12,
		RETURN = 13
	}

	public static class BytecodeUtil {
		// Instruction field extraction helpers
		public static Byte OP(UInt32 instruction) => (Byte)((instruction >> 24) & 0xFF);
		
		// 8-bit field extractors
		public static Byte Au(UInt32 instruction) => (Byte)((instruction >> 16) & 0xFF);
		public static Byte Bu(UInt32 instruction) => (Byte)((instruction >> 8) & 0xFF);
		public static Byte Cu(UInt32 instruction) => (Byte)(instruction & 0xFF);
		
		public static SByte As(UInt32 instruction) => (SByte)Au(instruction);
		public static SByte Bs(UInt32 instruction) => (SByte)Bu(instruction);
		public static SByte Cs(UInt32 instruction) => (SByte)Cu(instruction);
		
		// 16-bit field extractors
		public static UInt16 BCu(UInt32 instruction) => (UInt16)(instruction & 0xFFFF);
		public static Int16 BCs(UInt32 instruction) => (Int16)BCu(instruction);
		
		// 24-bit field extractors
		public static UInt32 ABCu(UInt32 instruction) => instruction & 0xFFFFFF;
		public static Int32 ABCs(UInt32 instruction) {
			UInt32 value = ABCu(instruction);
			// If bit 23 is set (sign bit), extend the sign to upper 8 bits
			if ((value & 0x800000) != 0) {
				return (Int32)(value | 0xFF000000);
			}
			return (Int32)value;
		}
		
		// Instruction encoding helpers
		public static UInt32 INS(Opcode op) => (UInt32)((Byte)op << 24);
		public static UInt32 INS_AB(Opcode op, Byte a, Int16 bc) => (UInt32)(((Byte)op << 24) | (a << 16) | ((UInt16)bc));
		public static UInt32 INS_ABC(Opcode op, Byte a, Byte b, Byte c) => (UInt32)(((Byte)op << 24) | (a << 16) | (b << 8) | c);
		
		// Conversion to/from opcode mnemonics (names)
		public static String ToMnemonic(Opcode opcode) {
			switch (opcode) {
				case Opcode.NOOP:          return "NOOP";
				case Opcode.LOAD_rA_rB:    return "LOAD_rA_rB";
				case Opcode.LOAD_rA_iBC:   return "LOAD_rA_iBC";
				case Opcode.LOAD_rA_kBC:   return "LOAD_rA_kBC";
				case Opcode.ADD_rA_rB_rC:  return "ADD_rA_rB_rC";
				case Opcode.SUB_rA_rB_rC:  return "SUB_rA_rB_rC";
				case Opcode.MULT_rA_rB_rC: return "MULT_rA_rB_rC";
				case Opcode.DIV_rA_rB_rC:  return "DIV_rA_rB_rC";
				case Opcode.MOD_rA_rB_rC:  return "MOD_rA_rB_rC";
				case Opcode.JUMP_iABC:     return "JUMP_iABC";
				case Opcode.IFLT_rA_rB:    return "IFLT_rA_rB";
				case Opcode.IFLT_rA_iBC:   return "IFLT_rA_iBC";
				case Opcode.CALLF_iA_iBC:  return "CALLF_iA_iBC";
				case Opcode.RETURN:        return "RETURN";
				default:
					return "Unknown opcode";
			}
		}
		
		public static Opcode FromMnemonic(String s) {
			if (s == "NOOP")          return Opcode.NOOP;
			if (s == "LOAD_rA_rB")    return Opcode.LOAD_rA_rB;
			if (s == "LOAD_rA_iBC")   return Opcode.LOAD_rA_iBC;
			if (s == "LOAD_rA_kBC")   return Opcode.LOAD_rA_kBC;
			if (s == "ADD_rA_rB_rC")  return Opcode.ADD_rA_rB_rC;
			if (s == "SUB_rA_rB_rC")  return Opcode.SUB_rA_rB_rC;
			if (s == "MULT_rA_rB_rC") return Opcode.MULT_rA_rB_rC;
			if (s == "DIV_rA_rB_rC")  return Opcode.DIV_rA_rB_rC;
			if (s == "MOD_rA_rB_rC")  return Opcode.MOD_rA_rB_rC;
			if (s == "JUMP_iABC")     return Opcode.JUMP_iABC;
			if (s == "IFLT_rA_rB")    return Opcode.IFLT_rA_rB;
			if (s == "IFLT_rA_iBC")   return Opcode.IFLT_rA_iBC;
			if (s == "CALLF_iA_iBC")  return Opcode.CALLF_iA_iBC;
			if (s == "RETURN")        return Opcode.RETURN;
			return Opcode.NOOP;
		}
	}
	
	// Function definition: code, constants, and how many registers it needs
	public class FuncDef {
		public String Name = "";
		public List<UInt32> Code = new List<UInt32>();
		public List<Value> Constants = new List<Value>();
		public UInt16 MaxRegs = 0; // frame reservation size
		
		// Conversion to bool: returns true if function has a name
		/*** BEGIN H_ONLY ***
		public: operator bool() { return Name != ""; }
		*** END H_ONLY ***/
		//*** BEGIN CS_ONLY ***
		public static implicit operator bool(FuncDef funcDef) {
			return funcDef != null && !String.IsNullOrEmpty(funcDef.Name);
		}
		//*** END CS_ONLY ***
	}

}


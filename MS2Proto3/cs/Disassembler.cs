using System;
using System.Collections.Generic;
// CPP: #include "Bytecode.g.h"
// CPP: #include "StringUtils.g.h"

namespace MiniScript {

	public static class Disassembler {
	
		// Return the short pseudo-opcode for the given instruction
		// (e.g.: LOAD instead of LOAD_rA_iBC, etc.)
		public static String AssemOp(Opcode opcode) {
			switch (opcode) {
				case Opcode.NOOP:         return "NOOP";
				case Opcode.LOAD_rA_rB:
				case Opcode.LOAD_rA_iBC:
				case Opcode.LOAD_rA_kBC:  return "LOAD";
				case Opcode.ADD_rA_rB_rC: return "ADD";
				case Opcode.SUB_rA_rB_rC: return "SUB";
				case Opcode.JUMP_iABC:    return "JUMP";
				case Opcode.IFLT_rA_rB:
				case Opcode.IFLT_rA_iBC:  return "IFLT";
				case Opcode.CALLF_iA_iBC: return "CALLF";
				case Opcode.RETURN:       return "RETURN";
				default:
					return "Unknown opcode";
			}		
		}
		
		public static String ToString(UInt32 instruction) {
			Opcode opcode = (Opcode)BytecodeUtil.OP(instruction);
			String mnemonic = AssemOp(opcode);
			mnemonic = (mnemonic + "    ").Left(6);
			
			// In the following switch, we group opcodes according
			// to their operand usage.
			switch (opcode) {
				// No operands:
				case Opcode.NOOP:
				case Opcode.RETURN:
					return mnemonic;
				
				// One Operand:
        		// iABC
        		case Opcode.JUMP_iABC:
        			return StringUtils.Format("{0} {1}",
        				mnemonic,
        				(Int32)BytecodeUtil.ABC(instruction));
        		
        		// Two Operands:
				// rA, rB
				case Opcode.LOAD_rA_rB:
        		case Opcode.IFLT_rA_rB:
					return StringUtils.Format("{0} r{1}, r{2}",
						mnemonic,
						(Int32)BytecodeUtil.A(instruction),
						(Int32)BytecodeUtil.B(instruction));
				// rA, kBC
        		case Opcode.LOAD_rA_kBC:
        			return StringUtils.Format("{0} r{1}, k{2}",
        				mnemonic,
        				(Int32)BytecodeUtil.A(instruction),
        				(Int32)BytecodeUtil.BCu(instruction));
        		// rA, iBC
        		case Opcode.LOAD_rA_iBC:
        		case Opcode.IFLT_rA_iBC:
        			return StringUtils.Format("{0} r{1}, {2}",
        				mnemonic,
        				(Int32)BytecodeUtil.A(instruction),
        				(Int32)BytecodeUtil.BC(instruction));
        		// iA, iBC
				case Opcode.CALLF_iA_iBC:
        			return StringUtils.Format("{0} {1}, {2}",
        				mnemonic,
        				(Int32)BytecodeUtil.A(instruction),
        				(Int32)BytecodeUtil.BC(instruction));
        		
        		// Three Operands:
        		// rA, rB, rC
        		case Opcode.ADD_rA_rB_rC:
        		case Opcode.SUB_rA_rB_rC:
        			return StringUtils.Format("{0} r{1}, r{2}, r{3}",
        				mnemonic,
        				(Int32)BytecodeUtil.A(instruction),
        				(Int32)BytecodeUtil.B(instruction),
        				(Int32)BytecodeUtil.C(instruction));
				default:
					return new String("??? ") + StringUtils.ToHex(instruction);
			}
		}
	
	}

}

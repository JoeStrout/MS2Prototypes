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
				case Opcode.NOOP:          return "NOOP";
				case Opcode.LOAD_rA_rB:
				case Opcode.LOAD_rA_iBC:
				case Opcode.LOAD_rA_kBC:   return "LOAD";
				case Opcode.ADD_rA_rB_rC:  return "ADD";
				case Opcode.MULT_rA_rB_rC: return "MULT";
				case Opcode.DIV_rA_rB_rC:  return "DIV";
				case Opcode.MOD_rA_rB_rC:  return "MOD";
				case Opcode.LIST_rA_iBC:   return "LIST";
				case Opcode.PUSH_rA_rB:    return "PUSH";
				case Opcode.INDEX_rA_rB_rC: return "INDEX";
				case Opcode.IDXSET_rA_rB_rC: return "IDXSET";
				case Opcode.SUB_rA_rB_rC:  return "SUB";
				case Opcode.JUMP_iABC:     return "JUMP";
				case Opcode.LT_rA_rB_rC:
				case Opcode.LT_rA_rB_iC:
				case Opcode.LT_rA_iB_rC:   return "LT";
				case Opcode.LE_rA_rB_rC:
				case Opcode.LE_rA_rB_iC:
				case Opcode.LE_rA_iB_rC:   return "LE";
				case Opcode.EQ_rA_rB_rC:
				case Opcode.EQ_rA_rB_iC:   return "EQ";
				case Opcode.NE_rA_rB_rC:
				case Opcode.NE_rA_rB_iC:   return "NE";
				case Opcode.BRTRUE_rA_iBC: return "BCTRUE";
				case Opcode.BRFALSE_rA_iBC:return "BCFALSE";
				case Opcode.BRLT_rA_rB_iC:
				case Opcode.BRLT_rA_iB_iC:
				case Opcode.BRLT_iA_rB_iC: return "BRLT";
				case Opcode.BRLE_rA_rB_iC:
				case Opcode.BRLE_rA_iB_iC:
				case Opcode.BRLE_iA_rB_iC: return "BRLE";
				case Opcode.BREQ_rA_rB_iC:
				case Opcode.BREQ_rA_iB_iC: return "BREQ";
				case Opcode.BRNE_rA_rB_iC:
				case Opcode.BRNE_rA_iB_iC: return "BRNE";
				case Opcode.IFLT_rA_rB:
				case Opcode.IFLT_rA_iBC:
				case Opcode.IFLT_iAB_rC:   return "IFLT";
				case Opcode.IFLE_rA_rB:
				case Opcode.IFLE_rA_iBC:
				case Opcode.IFLE_iAB_rC:   return "IFLE";
				case Opcode.IFEQ_rA_rB:
				case Opcode.IFEQ_rA_iBC:   return "IFEQ";
				case Opcode.IFNE_rA_rB:
				case Opcode.IFNE_rA_iBC:   return "IFNE";
				case Opcode.CALLF_iA_iBC:  return "CALLF";
				case Opcode.RETURN:        return "RETURN";
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
        				BytecodeUtil.ABCs(instruction));
        		
        		// Two Operands:
				// rA, rB
				case Opcode.LOAD_rA_rB:
				case Opcode.PUSH_rA_rB:
        		case Opcode.IFLT_rA_rB:
				case Opcode.IFLE_rA_rB:
				case Opcode.IFEQ_rA_rB:
				case Opcode.IFNE_rA_rB:
					return StringUtils.Format("{0} r{1}, r{2}",
						mnemonic,
						(Int32)BytecodeUtil.Au(instruction),
						(Int32)BytecodeUtil.Bu(instruction));
				// rA, kBC
        		case Opcode.LOAD_rA_kBC:
        			return StringUtils.Format("{0} r{1}, k{2}",
        				mnemonic,
        				(Int32)BytecodeUtil.Au(instruction),
        				(Int32)BytecodeUtil.BCu(instruction));
        		// rA, iBC
        		case Opcode.LOAD_rA_iBC:
        		case Opcode.LIST_rA_iBC:
        		case Opcode.IFLT_rA_iBC:
				case Opcode.IFLE_rA_iBC:
				case Opcode.IFEQ_rA_iBC:
				case Opcode.IFNE_rA_iBC:
				case Opcode.BRTRUE_rA_iBC:
				case Opcode.BRFALSE_rA_iBC:
        			return StringUtils.Format("{0} r{1}, {2}",
        				mnemonic,
        				(Int32)BytecodeUtil.Au(instruction),
        				(Int32)BytecodeUtil.BCs(instruction));
				// iAB, rC
        		case Opcode.IFLT_iAB_rC:
				case Opcode.IFLE_iAB_rC:
        			return StringUtils.Format("{0} {1}, r{2}",
        				mnemonic,
        				(Int32)BytecodeUtil.ABs(instruction),
        				(Int32)BytecodeUtil.Cu(instruction));
        		// iA, iBC
				case Opcode.CALLF_iA_iBC:
        			return StringUtils.Format("{0} {1}, {2}",
        				mnemonic,
        				(Int32)BytecodeUtil.As(instruction),
        				(Int32)BytecodeUtil.BCs(instruction));
        		
        		// Three Operands:
        		// rA, rB, rC
        		case Opcode.ADD_rA_rB_rC:
        		case Opcode.SUB_rA_rB_rC:
				case Opcode.MULT_rA_rB_rC:
				case Opcode.DIV_rA_rB_rC:
				case Opcode.MOD_rA_rB_rC:
				case Opcode.LT_rA_rB_rC:
				case Opcode.LE_rA_rB_rC:
				case Opcode.EQ_rA_rB_rC:
				case Opcode.NE_rA_rB_rC:
				case Opcode.INDEX_rA_rB_rC:
				case Opcode.IDXSET_rA_rB_rC:
        			return StringUtils.Format("{0} r{1}, r{2}, r{3}",
        				mnemonic,
        				(Int32)BytecodeUtil.Au(instruction),
        				(Int32)BytecodeUtil.Bu(instruction),
        				(Int32)BytecodeUtil.Cu(instruction));
				// rA, rB, iC
				case Opcode.LT_rA_rB_iC:
				case Opcode.LE_rA_rB_iC:
				case Opcode.EQ_rA_rB_iC:
				case Opcode.NE_rA_rB_iC:
				case Opcode.BRLT_rA_rB_iC:
				case Opcode.BRLE_rA_rB_iC:
				case Opcode.BREQ_rA_rB_iC:
				case Opcode.BRNE_rA_rB_iC:
					return StringUtils.Format("{0} r{1}, r{2}, {3}",
        				mnemonic,
        				(Int32)BytecodeUtil.Au(instruction),
        				(Int32)BytecodeUtil.Bu(instruction),
        				(Int32)BytecodeUtil.Cu(instruction));
				// rA, iB, iC
				case Opcode.BRLT_rA_iB_iC:
				case Opcode.BRLE_rA_iB_iC:
				case Opcode.BREQ_rA_iB_iC:
				case Opcode.BRNE_rA_iB_iC:
					return StringUtils.Format("{0} r{1}, {2}, {3}",
        				mnemonic,
        				(Int32)BytecodeUtil.Au(instruction),
        				(Int32)BytecodeUtil.Bu(instruction),
        				(Int32)BytecodeUtil.Cu(instruction));
				// iA, rB, iC
				case Opcode.BRLT_iA_rB_iC:
				case Opcode.BRLE_iA_rB_iC:
					return StringUtils.Format("{0} {1}, r{2}, {3}",
        				mnemonic,
        				(Int32)BytecodeUtil.Au(instruction),
        				(Int32)BytecodeUtil.Bu(instruction),
        				(Int32)BytecodeUtil.Cu(instruction));
				// rA, iB, rC
				case Opcode.LT_rA_iB_rC:
				case Opcode.LE_rA_iB_rC:
					return StringUtils.Format("{0} r{1}, {2}, r{3}",
        				mnemonic,
        				(Int32)BytecodeUtil.Au(instruction),
        				(Int32)BytecodeUtil.Bu(instruction),
        				(Int32)BytecodeUtil.Cu(instruction));
				default:
					return new String("??? ") + StringUtils.ToHex(instruction);
			}
		}
	
		// Disassemble the given function.  If detailed=true, include extra
		// details for debugging, like line numbers and instruction hex code.
		public static void Disassemble(FuncDef funcDef, List<String> output, Boolean detailed=true) {
			output.Add(StringUtils.Format("Constants ({0}):", funcDef.Constants.Count));
			for (Int32 i = 0; i < funcDef.Constants.Count; i++) {
				output.Add(StringUtils.Format("   {0}. {1}", i, funcDef.Constants[i]));
			}
			
			output.Add(StringUtils.Format("Instructions ({0}):", funcDef.Code.Count));
			for (Int32 i = 0; i < funcDef.Code.Count; i++) {				
				String s = ToString(funcDef.Code[i]);
				if (detailed) {
					s = StringUtils.ZeroPad(i, 4) + ":  "
					  + StringUtils.ToHex(funcDef.Code[i]) + " | "
					  + s;
				}
				output.Add(s);
			}
		}

		// Disassemble the whole program (list of functions).  If detailed=true, include 
		// extra details for debugging, like line numbers and instruction hex code.
		public static List<String> Disassemble(List<FuncDef> functions, Boolean detailed=true) {
			List<String> result = new List<String>();
			for (Int32 i = 0; i < functions.Count; i++) {
				result.Add(StringUtils.Format("{0} (function {1}):", functions[i].Name, i));
				Disassemble(functions[i], result, detailed);
				result.Add("");
			}
			return result;
		}

	}

}

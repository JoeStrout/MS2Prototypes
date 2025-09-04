using System;
using System.Collections.Generic;
// CPP: #include "Bytecode.g.h"
// CPP: #include "StringUtils.g.h"

namespace MiniScript {

	public class Assembler {

		public FuncDef Current; // function we are currently building

		public Assembler() {
			Current = new FuncDef();
		}

		public static List<String> GetTokens(String line) {
			// Clean the string, stripping off comment at '#',
			// and divide into tokens by whitespace and commas.  Example:
			//  "   LOAD r5, r6 # comment"  -->  ["LOAD", "r5", "r6"]
			
			// Parse line character by character, collecting tokens
			List<String> tokens = new List<String>();
			Int32 tokenStart = -1;
			
			for (Int32 i = 0; i < line.Length; i++) {
				Char c = line[i];
				
				// Stop at comment
				if (c == '#') break;
				
				// Check if this is a delimiter (whitespace or comma)
				if (c == ' ' || c == '\t' || c == ',') {
					// End current token if we have one
					if (tokenStart >= 0) {
						tokens.Add(line.Substring(tokenStart, i - tokenStart));
						tokenStart = -1;
					}
				} else {
					// Start new token if not already started
					if (tokenStart < 0) {
						tokenStart = i;
					}
				}
			}
			
			// Add final token if any
			if (tokenStart >= 0) {
				tokens.Add(line.Substring(tokenStart, line.Length - tokenStart));
			}
			return tokens;
		}
		
		public UInt32 AddLine(String line) {
			List<String> parts = GetTokens(line);
			
			// Empty line or comment-only line
			if (parts.Count == 0) return 0;
			
			String mnemonic = parts[0];
			UInt32 instruction = 0;
			
			if (mnemonic == "NOOP") {
				instruction = BytecodeUtil.INS(Opcode.NOOP);
				
			} else if (mnemonic == "LOAD") {
				if (parts.Count != 3) return 0; // error: wrong number of operands
				
				String destReg = parts[1];  // should be "r5" etc.
				String source = parts[2];   // "r6", "42", or "k20" 
				
				Byte dest = ParseRegister(destReg);
				
				if (source[0] == 'r') {
					// LOAD r2, r5  -->  LOAD_rA_rB
					Byte srcReg = ParseRegister(source);
					instruction = BytecodeUtil.INS_ABC(Opcode.LOAD_rA_rB, dest, srcReg, 0);
				} else if (source[0] == 'k') {
					// LOAD r3, k20  -->  LOAD_rA_kBC
					Int16 constIdx = ParseNumber(source.Substring(1));
					instruction = BytecodeUtil.INS_AB(Opcode.LOAD_rA_kBC, dest, constIdx);
				} else {
					// LOAD r6, 42  -->  LOAD_rA_iBC
					Int16 immediate = ParseNumber(source);
					instruction = BytecodeUtil.INS_AB(Opcode.LOAD_rA_iBC, dest, immediate);
				}
				
			} else if (mnemonic == "ADD") {
				if (parts.Count != 4) return 0; // ADD r5, r3, r4
				Byte dest = ParseRegister(parts[1]);
				Byte src1 = ParseRegister(parts[2]);
				Byte src2 = ParseRegister(parts[3]);
				instruction = BytecodeUtil.INS_ABC(Opcode.ADD_rA_rB_rC, dest, src1, src2);
				
			} else if (mnemonic == "SUB") {
				if (parts.Count != 4) return 0; // SUB r5, r3, r4
				Byte dest = ParseRegister(parts[1]);
				Byte src1 = ParseRegister(parts[2]);
				Byte src2 = ParseRegister(parts[3]);
				instruction = BytecodeUtil.INS_ABC(Opcode.SUB_rA_rB_rC, dest, src1, src2);
				
			} else if (mnemonic == "JUMP") {
				if (parts.Count != 2) return 0; // JUMP 42
				Int32 offset = ParseNumber(parts[1]);
				instruction = BytecodeUtil.INS(Opcode.JUMP_iABC) | (UInt32)(offset & 0xFFFFFF);
				
			} else if (mnemonic == "IFLT") {
				if (parts.Count == 3) {
					// IFLT r5, r3
					Byte reg1 = ParseRegister(parts[1]);
					Byte reg2 = ParseRegister(parts[2]);
					instruction = BytecodeUtil.INS_ABC(Opcode.IFLT_rA_rB, reg1, reg2, 0);
				} else if (parts.Count == 2) {
					// IFLT r5, 42
					// Actually this would be parsed differently, let me fix
					// For now, assume 2 register form
					return 0; // TODO: handle immediate form
				}
				
			} else if (mnemonic == "CALLF") {
				if (parts.Count != 3) return 0; // CALLF 5, 12 (reserve 5 regs, call func 12)
				Byte reserveRegs = (Byte)ParseNumber(parts[1]);
				Int16 funcIdx = ParseNumber(parts[2]);
				instruction = BytecodeUtil.INS_AB(Opcode.CALLF_iA_iBC, reserveRegs, funcIdx);
				
			} else if (mnemonic == "RETURN") {
				instruction = BytecodeUtil.INS(Opcode.RETURN);
			}
			
			// Add instruction to current function
			Current.Code.Add(instruction);
			
			return instruction;
		}
		
		// Helper to parse register like "r5" -> 5
		private static Byte ParseRegister(String reg) {
			if (reg.Length < 2 || reg[0] != 'r') return 0;
			return (Byte)ParseNumber(reg.Substring(1));
		}
		
		// Helper to parse number (handles negative numbers)
		private static Int16 ParseNumber(String num) {
			// Simple number parsing - could be enhanced
			Int32 result = 0;
			Boolean negative = false;
			Int32 start = 0;
			
			if (num.Length > 0 && num[0] == '-') {
				negative = true;
				start = 1;
			}
			
			for (Int32 i = start; i < num.Length; i++) {
				if (num[i] >= '0' && num[i] <= '9') {
					result = result * 10 + (num[i] - '0');
				}
			}
			
			return (Int16)(negative ? -result : result);
		}

	}
}

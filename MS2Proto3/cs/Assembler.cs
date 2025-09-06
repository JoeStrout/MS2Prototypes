using System;
using System.Collections.Generic;
// CPP: #include "Bytecode.g.h"
// CPP: #include "StringUtils.g.h"

namespace MiniScript {

	public class Assembler {

		public FuncDef Current; // function we are currently building
		private List<String> _labelNames; // label names
		private List<Int32> _labelAddresses; // corresponding instruction addresses

		public Assembler() {
			Current = new FuncDef();
			_labelNames = new List<String>();
			_labelAddresses = new List<Int32>();
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
		
		// Assemble a single source line, add to our current function,
		// and also return its value (mainly for unit testing).
		public UInt32 AddLine(String line) {
			// Break into tokens (stripping whitespace, commas, and comments)
			List<String> parts = GetTokens(line);
			
			// Check if first token is a label and remove it
			if (parts.Count > 0 && IsLabel(parts[0])) parts.RemoveAt(0);

			// If there is no instruction on this line, return 0
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
				if (parts.Count != 2) return 0; // JUMP labelName or JUMP 42
				String target = parts[1];
				Int32 offset;
				
				// Check if target is a label or a number
				Int32 labelAddr = FindLabelAddress(target);
				if (labelAddr >= 0) {
					// It's a label - calculate relative offset from next instruction
					offset = labelAddr - (Current.Code.Count + 1);
				} else {
					// It's a number
					offset = ParseNumber(target);
				}
				instruction = BytecodeUtil.INS(Opcode.JUMP_iABC) | (UInt32)(offset & 0xFFFFFF);
				
			} else if (mnemonic == "IFLT") {
				if (parts.Count != 3) return 0; // error: wrong number of operands
				
				Byte reg1 = ParseRegister(parts[1]);
				
				if (parts[2][0] == 'r') {
					// IFLT r5, r3  -->  IFLT_rA_rB
					Byte reg2 = ParseRegister(parts[2]);
					instruction = BytecodeUtil.INS_ABC(Opcode.IFLT_rA_rB, reg1, reg2, 0);
				} else {
					// IFLT r5, 42  -->  IFLT_rA_iBC
					Int16 immediate = ParseNumber(parts[2]);
					instruction = BytecodeUtil.INS_AB(Opcode.IFLT_rA_iBC, reg1, immediate);
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

		// Helper to determine if a token is a label (ends with colon)
		private static Boolean IsLabel(String token) {
			return token.Length > 1 && token[token.Length - 1] == ':';
		}

		// Helper to find the address of a label
		private Int32 FindLabelAddress(String labelName) {
			for (Int32 i = 0; i < _labelNames.Count; i++) {
				if (_labelNames[i] == labelName) return _labelAddresses[i];
			}
			return -1; // not found
		}

		// Two-pass assembly: first pass collects labels, second pass assembles with label resolution
		public void Assemble(List<String> sourceLines) {
			// Clear any previous state
			Current = new FuncDef();
			_labelNames.Clear();
			_labelAddresses.Clear();

			// First pass: collect label positions
			Int32 instructionAddress = 0;
			for (Int32 i = 0; i < sourceLines.Count; i++) {
				List<String> tokens = GetTokens(sourceLines[i]);
				if (tokens.Count == 0) continue; // empty line or comment only

				// Check if first token is a label
				if (IsLabel(tokens[0])) {
					String labelName = tokens[0].Substring(0, tokens[0].Length - 1);
					_labelNames.Add(labelName);
					_labelAddresses.Add(instructionAddress);
				}
				
				// Check if there's an instruction on this line
				// (either no label, or "NOOP", or label + instruction)
				if (!IsLabel(tokens[0]) || tokens[0]=="NOOP" || tokens.Count > 1) {
					instructionAddress++;
				}
			}

			// Second pass: assemble instructions with label resolution
			for (Int32 i = 0; i < sourceLines.Count; i++) {
				AddLine(sourceLines[i]);
			}
		}

	}
}

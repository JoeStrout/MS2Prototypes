using System;
using System.Collections.Generic;
// CPP: #include "nanbox.h"
// CPP: #include "strings.h"
// CPP: #include "Bytecode.g.h"
// CPP: #include "StringUtils.g.h"

using static MiniScript.ValueHelpers;

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
			//  "LOAD r1, \"Hello world\""  -->  ["LOAD", "r1", "\"Hello world\""]
			
			// Parse line character by character, collecting tokens
			List<String> tokens = new List<String>();
			Int32 tokenStart = -1;
			Boolean inQuotes = false;
			
			for (Int32 i = 0; i < line.Length; i++) {
				Char c = line[i];
				
				// Stop at comment (but not inside quotes)
				if (c == '#' && !inQuotes) break;
				
				// Handle quote characters
				if (c == '"') {
					if (tokenStart < 0) {
						// Start of quoted token
						tokenStart = i;
						inQuotes = true;
					} else if (inQuotes) {
						// End of quoted token
						tokens.Add(line.Substring(tokenStart, i - tokenStart + 1));
						tokenStart = -1;
						inQuotes = false;
					}
					continue;
				}
				
				// Check if this is a delimiter (whitespace or comma), but not inside quotes
				if ((c == ' ' || c == '\t' || c == ',') && !inQuotes) {
					// End current token if we have one
					if (tokenStart >= 0) {
						tokens.Add(line.Substring(tokenStart, i - tokenStart));
						tokenStart = -1;
					}
				} else {
					// Start new token if not already started and not already in quotes
					if (tokenStart < 0 && !inQuotes) {
						tokenStart = i;
					}
				}
			}
			
			// Add final token if any
			if (tokenStart >= 0) {
				if (inQuotes) {
					// Unclosed quote - include the rest of the line
					tokens.Add(line.Substring(tokenStart, line.Length - tokenStart));
				} else {
					tokens.Add(line.Substring(tokenStart, line.Length - tokenStart));
				}
			}
			return tokens;
		}
		
		private UInt32 Error(String errMsg, String mnemonic, String line) {
			IOHelper.Print(StringUtils.Format("ERROR: {0}", errMsg));
			IOHelper.Print(StringUtils.Format("on {0} instruction (source line: {1})",
					mnemonic, line));
			return 0;
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
				if (parts.Count != 3) return Error("Syntax error", mnemonic, line);
				
				String destReg = parts[1];  // should be "r5" etc.
				String source = parts[2];   // "r6", "42", "3.14", "hello", or "k20" 
								
				Byte dest = ParseRegister(destReg);
				
				if (source[0] == 'r') {
					// LOAD r2, r5  -->  LOAD_rA_rB
					Byte srcReg = ParseRegister(source);
					instruction = BytecodeUtil.INS_ABC(Opcode.LOAD_rA_rB, dest, srcReg, 0);
				} else if (source[0] == 'k') {
					// LOAD r3, k20  -->  LOAD_rA_kBC (explicit constant reference)
					Int16 constIdx = ParseNumber(source.Substring(1));
					instruction = BytecodeUtil.INS_AB(Opcode.LOAD_rA_kBC, dest, constIdx);
				} else if (NeedsConstant(source)) {
					// String literals, floats, or large integers -> add to constants table
					Value constantValue = ParseAsConstant(source);
					Int32 constIdx = AddConstant(constantValue);
					instruction = BytecodeUtil.INS_AB(Opcode.LOAD_rA_kBC, dest, (Int16)constIdx);
				} else {
					// Small integer that fits in Int16 -> use immediate form
					Int16 immediate = ParseNumber(source);
					instruction = BytecodeUtil.INS_AB(Opcode.LOAD_rA_iBC, dest, immediate);
				}
				
			} else if (mnemonic == "ADD") {
				if (parts.Count != 4) return Error("Syntax error", mnemonic, line);
				Byte dest = ParseRegister(parts[1]);
				Byte src1 = ParseRegister(parts[2]);
				Byte src2 = ParseRegister(parts[3]);
				instruction = BytecodeUtil.INS_ABC(Opcode.ADD_rA_rB_rC, dest, src1, src2);
				
			} else if (mnemonic == "SUB") {
				if (parts.Count != 4) return Error("Syntax error", mnemonic, line);
				Byte dest = ParseRegister(parts[1]);
				Byte src1 = ParseRegister(parts[2]);
				Byte src2 = ParseRegister(parts[3]);
				instruction = BytecodeUtil.INS_ABC(Opcode.SUB_rA_rB_rC, dest, src1, src2);
				
			} else if (mnemonic == "JUMP") {
				if (parts.Count != 2) return Error("Syntax error", mnemonic, line);
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
				if (parts.Count != 3) return Error("Syntax error", mnemonic, line);
				
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
				if (parts.Count != 3) return Error("Syntax error", mnemonic, line);
				Byte reserveRegs = (Byte)ParseNumber(parts[1]);
				Int16 funcIdx = ParseNumber(parts[2]);
				instruction = BytecodeUtil.INS_AB(Opcode.CALLF_iA_iBC, reserveRegs, funcIdx);
				
			} else if (mnemonic == "RETURN") {
				instruction = BytecodeUtil.INS(Opcode.RETURN);
			
			} else {
				return Error("Unknown opcode", mnemonic, line);
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

		// Helper to add a constant to the constants table and return its index
		private Int32 AddConstant(Value value) {
			Current.Constants.Add(value);
			return Current.Constants.Count - 1;
		}

		// Helper to check if a token is a string literal (surrounded by quotes)
		private static Boolean IsStringLiteral(String token) {
			return token.Length >= 2 && token[0] == '"' && token[token.Length - 1] == '"';
		}

		// Helper to check if a token needs to be stored as a constant
		// (string literals, floating point numbers, or integers too large for Int16)
		private static Boolean NeedsConstant(String token) {
			if (IsStringLiteral(token)) return true;
			
			// Check if it contains a decimal point (floating point number)
			if (token.Contains(".")) return true;
			
			// Check if it's an integer too large for Int16
			Int32 value = 0;
			Boolean negative = false;
			Int32 start = 0;
			
			if (token.Length > 0 && token[0] == '-') {
				negative = true;
				start = 1;
			}
			
			// Parse the integer
			for (Int32 i = start; i < token.Length; i++) {
				if (token[i] < '0' || token[i] > '9') return false; // Not a number
				value = value * 10 + (token[i] - '0');
			}
			
			Int32 finalValue = negative ? -value : value;
			return finalValue < -32768 || finalValue > 32767;
		}

		// Helper to create a Value from a token
		private static Value ParseAsConstant(String token) {
			if (IsStringLiteral(token)) {
				// Remove quotes and create string value
				String content = token.Substring(1, token.Length - 2);
				return make_string(content); // CPP: return make_string(content.c_str());
			}
			
			// Check if it contains a decimal point (floating point number)
			if (token.Contains(".")) {
				// Simple double parsing (basic implementation)
				Double doubleValue = ParseDouble(token);
				return make_double(doubleValue);
			}
			
			// Parse as integer
			Int32 intValue = ParseNumber(token);
			return make_int(intValue);
		}

		// Helper to parse a double from a string (basic implementation)
		private static Double ParseDouble(String str) {
			// Find the decimal point
			Int32 dotPos = -1;
			for (Int32 i = 0; i < str.Length; i++) {
				if (str[i] == '.') {
					dotPos = i;
					break;
				}
			}
			
			if (dotPos == -1) {
				// No decimal point, parse as integer
				return (Double)ParseNumber(str);
			}
			
			// Parse integer part
			String intPart = str.Substring(0, dotPos);
			Double result = (Double)ParseNumber(intPart);
			
			// Parse fractional part
			String fracPart = str.Substring(dotPos + 1);
			if (fracPart.Length > 0) {
				Double fracValue = (Double)ParseNumber(fracPart);
				Double divisor = 1.0;
				for (Int32 i = 0; i < fracPart.Length; i++) {
					divisor *= 10.0;
				}
				result += fracValue / divisor;
			}
			
			// Handle negative numbers
			if (str.Length > 0 && str[0] == '-') {
				result = -result;
			}
			
			return result;
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

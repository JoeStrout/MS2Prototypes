using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
// CPP: #include "value.h"
// CPP: #include "Bytecode.g.h"
// CPP: #include "IOHelper.g.h"
// CPP: #include "Disassembler.g.h"
// CPP: #include "StringUtils.g.h"
// CPP: #include "dispatch_macros.h"

using static MiniScript.ValueHelpers;

namespace MiniScript {

	// Call stack frame (return info)
	public struct CallInfo {
		public Int32 ReturnPC;		   // where to continue in caller (PC index)
		public Int32 ReturnBase;		 // caller's base pointer (stack index)

		public CallInfo(Int32 returnPC, Int32 returnBase) {
			ReturnPC = returnPC;
			ReturnBase = returnBase;
		}
	}

	// VM state
	public class VM {
		private List<Value> stack;
		
		private List<CallInfo> callStack;
		private Int32 callStackTop;	   // Index of next free call stack slot
		
		private List<FuncDef> functions; // functions addressed by CALLF

		public VM() {
			InitVM(1024, 256);
		}
		
		public VM(Int32 stackSlots, Int32 callSlots) {
			InitVM(stackSlots, callSlots);
		}

		private void InitVM(Int32 stackSlots, Int32 callSlots) {
			stack = new List<Value>();
			callStack = new List<CallInfo>();
			functions = new List<FuncDef>();
			callStackTop = 0;
			
			// Initialize stack with null values
			for (Int32 i = 0; i < stackSlots; i++) {
				stack.Add(make_null());
			}
			
			
			// Pre-allocate call stack capacity
			for (Int32 i = 0; i < callSlots; i++) {
				callStack.Add(new CallInfo(0, 0));
			}
		}

		public void RegisterFunction(FuncDef funcDef) {
			functions.Add(funcDef);
		}

		// Run a program: store all functions, find @main, and execute it
		public Value Run(List<FuncDef> allFunctions) {
			// Store all functions for CALLF instructions, and find @main
			FuncDef mainFunc = new FuncDef();
			functions.Clear();
			for (Int32 i = 0; i < allFunctions.Count; i++) {
				functions.Add(allFunctions[i]);
				if (functions[i].Name == "@main") mainFunc = functions[i];
			}
			
			if (!mainFunc) {
				IOHelper.Print("ERROR: No @main function found in VM.Run");
				return make_null();
			}
			
			// Execute @main
			IOHelper.Print(StringUtils.Format("Executing {0} out of {1} functions", mainFunc.Name, functions.Count));
			return Execute(mainFunc);
		}

		public Value Execute(FuncDef entry) {
			return Execute(entry, 0);
		}
		
		public Value Execute(FuncDef entry, UInt32 maxCycles) {
			// Basic validation - simplified for C++ transpilation
			if (entry.Code.Count == 0) {
				IOHelper.Print("Entry function has no code");
				return make_null();
			}

			// Current frame state
			Int32 baseIndex = 0;			  // entry executes at stack base
			Int32 pc = 0;					 // start at entry code
			UInt32 cycleCount = 0;
			bool debug = true;			 // Set to true for debug output

			EnsureFrame(baseIndex, entry.MaxRegs);

/*** BEGIN CPP_ONLY ***
#if VM_USE_COMPUTED_GOTO
			static void* const vm_labels[(int)Opcode::OP__COUNT] = { VM_OPCODES(VM_LABEL_LIST) };
			IOHelper::Print("(Running with computed-goto dispatch)");
#else
			IOHelper::Print("(Running with switch-based dispatch)");
#endif
*** END CPP_ONLY ***/

			while (true) {
				// CPP: VM_DISPATCH_TOP();
				cycleCount++;
				if (maxCycles > 0 && cycleCount > maxCycles) {
					IOHelper.Print("VM: Hit cycle limit");
					return make_null();
				}

				if (pc >= entry.Code.Count) {
					IOHelper.Print("VM: PC out of bounds");
					return make_null();
				}

				UInt32 instruction = entry.Code[pc++];
				
				if (debug) {
					// Debug output disabled for C++ transpilation
					IOHelper.Print(StringUtils.Format("VM instruction: {0}; r0:{1}, r1:{2}, r2:{3}",
						Disassembler.ToString(instruction), 
						stack[baseIndex+0], stack[baseIndex+1], stack[baseIndex+2]));
				}

				Opcode opcode = (Opcode)BytecodeUtil.OP(instruction);
				
				switch (opcode) { // CPP: VM_DISPATCH_BEGIN();
				
					case Opcode.NOOP: { // CPP: VM_CASE(NOOP) {
						// No operation
						break; // CPP: VM_NEXT();
					}

					case Opcode.LOAD_rA_rB: { // CPP: VM_CASE(LOAD_rA_rB) {
						// R[A] = R[B] (equivalent to MOVE)
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						stack[baseIndex + a] = stack[baseIndex + b];
						break; // CPP: VM_NEXT();
					}

					case Opcode.LOAD_rA_iBC: { // CPP: VM_CASE(LOAD_rA_iBC) {
						// R[A] = BC (signed 16-bit immediate as integer)
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						stack[baseIndex + a] = make_int(bc);
						break; // CPP: VM_NEXT();
					}

					case Opcode.LOAD_rA_kBC: { // CPP: VM_CASE(LOAD_rA_kBC) {
						// R[A] = constants[BC]
						Byte a = BytecodeUtil.Au(instruction);
						UInt16 constIdx = BytecodeUtil.BCu(instruction);
						if (constIdx >= entry.Constants.Count) {
							IOHelper.Print("LOAD_rA_kBC: invalid constant index");
							return make_null();
						}
						stack[baseIndex + a] = entry.Constants[constIdx];
						break; // CPP: VM_NEXT();
					}

					case Opcode.ADD_rA_rB_rC: { // CPP: VM_CASE(ADD_rA_rB_rC) {
						// R[A] = R[B] + R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						stack[baseIndex + a] = value_add(stack[baseIndex + b], stack[baseIndex + c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.SUB_rA_rB_rC: { // CPP: VM_CASE(SUB_rA_rB_rC) {
						// R[A] = R[B] - R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						stack[baseIndex + a] = value_sub(stack[baseIndex + b], stack[baseIndex + c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.MULT_rA_rB_rC: { // CPP: VM_CASE(MULT_rA_rB_rC) {
						// R[A] = R[B] * R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						stack[baseIndex + a] = value_mult(stack[baseIndex + b], stack[baseIndex + c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.DIV_rA_rB_rC: { // CPP: VM_CASE(DIV_rA_rB_rC) {
						// R[A] = R[B] * R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						stack[baseIndex + a] = value_div(stack[baseIndex + b], stack[baseIndex + c]);
						break; // CPP: VM_NEXT();
					}

                    case Opcode.MOD_rA_rB_rC: { // CPP: VM_CASE(MOD_rA_rB_rC) {
						// R[A] = R[B] % R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						stack[baseIndex + a] = value_mod(stack[baseIndex + b], stack[baseIndex + c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.JUMP_iABC: { // CPP: VM_CASE(JUMP_iABC) {
						// Jump by signed 24-bit ABC offset from current PC
						Int32 offset = BytecodeUtil.ABCs(instruction);
						pc += offset;
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRTRUE_rA_iBC: { // CPP: VM_CASE(BRTRUE_rA_iBC) {
						Byte a = BytecodeUtil.Au(instruction);
						Int32 offset = BytecodeUtil.BCs(instruction);
						if(is_truthy(stack[baseIndex + a])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRFALSE_rA_iBC: { // CPP: VM_CASE(BRFALSE_rA_iBC) {
						Byte a = BytecodeUtil.Au(instruction);
						Int32 offset = BytecodeUtil.BCs(instruction);
						if(!is_truthy(stack[baseIndex + a])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLT_rA_rB_iC: { // CPP: VM_CASE(BRLT_rA_rB_iC) {
						// if R[A] < R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_lt(stack[baseIndex + a], stack[baseIndex + b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLT_rA_iB_iC: { // CPP: VM_CASE(BRLT_rA_iB_iC) {
						// if R[A] < B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_lt(stack[baseIndex + a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLT_iA_rB_iC: { // CPP: VM_CASE(BRLT_iA_rB_iC) {
						// if A (immediate) < R[B] then jump offset C.
						SByte a = BytecodeUtil.As(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_lt(make_int(a), stack[baseIndex + b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLE_rA_rB_iC: { // CPP: VM_CASE(BRLE_rA_rB_iC) {
						// if R[A] <= R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_le(stack[baseIndex + a], stack[baseIndex + b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLE_rA_iB_iC: { // CPP: VM_CASE(BRLE_rA_iB_iC) {
						// if R[A] <= B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_le(stack[baseIndex + a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLE_iA_rB_iC: { // CPP: VM_CASE(BRLE_iA_rB_iC) {
						// if A (immediate) <= R[B] then jump offset C.
						SByte a = BytecodeUtil.As(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_le(make_int(a), stack[baseIndex + b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BREQ_rA_rB_iC: { // CPP: VM_CASE(BREQ_rA_rB_iC) {
						// if R[A] == R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_equal(stack[baseIndex + a], stack[baseIndex + b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BREQ_rA_iB_iC: { // CPP: VM_CASE(BREQ_rA_iB_iC) {
						// if R[A] == B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(value_equal(stack[baseIndex + a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRNE_rA_rB_iC: { // CPP: VM_CASE(BRNE_rA_rB_iC) {
						// if R[A] != R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(!value_equal(stack[baseIndex + a], stack[baseIndex + b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRNE_rA_iB_iC: { // CPP: VM_CASE(BRNE_rA_iB_iC) {
						// if R[A] != B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if(!value_equal(stack[baseIndex + a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.IFLT_rA_rB: { // CPP: VM_CASE(IFLT_rA_rB) {
						// if R[A] < R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (!value_lt(stack[baseIndex + a], stack[baseIndex + b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.IFLT_rA_iBC: { // CPP: VM_CASE(IFLT_rA_iBC) {
						// if R[A] < BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (!value_lt(stack[baseIndex + a], make_int(bc))) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFLT_iAB_rC: { // CPP: VM_CASE(IFLT_iAB_rC) {
						// if AB (immediate) < R[C] is false, skip next instruction
						short ab = BytecodeUtil.ABs(instruction);
                        Byte c = BytecodeUtil.Cu(instruction);
						if (!value_lt(make_int(ab), stack[baseIndex + c])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFLE_rA_rB: { // CPP: VM_CASE(IFLE_rA_rB) {
						// if R[A] <= R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (!value_le(stack[baseIndex + a], stack[baseIndex + b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.IFLE_rA_iBC: { // CPP: VM_CASE(IFLE_rA_iBC) {
						// if R[A] <= BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (!value_le(stack[baseIndex + a], make_int(bc))) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFLE_iAB_rC: { // CPP: VM_CASE(IFLE_iAB_rC) {
						// if AB (immediate) <= R[C] is false, skip next instruction
						short ab = BytecodeUtil.ABs(instruction);
                        Byte c = BytecodeUtil.Cu(instruction);
						if (!value_le(make_int(ab), stack[baseIndex + c])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFEQ_rA_rB: { // CPP: VM_CASE(IFEQ_rA_rB) {
						// if R[A] == R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (!value_equal(stack[baseIndex + a], stack[baseIndex + b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFEQ_rA_iBC: { // CPP: VM_CASE(IFEQ_rA_iBC) {
						// if R[A] == BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (!value_equal(stack[baseIndex + a], make_int(bc))) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFNE_rA_rB: { // CPP: VM_CASE(IFNE_rA_rB) {
						// if R[A] != R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (value_equal(stack[baseIndex + a], stack[baseIndex + b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFNE_rA_iBC: { // CPP: VM_CASE(IFNE_rA_iBC) {
						// if R[A] != BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (value_equal(stack[baseIndex + a], make_int(bc))) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.CALLF_iA_iBC: { // CPP: VM_CASE(CALLF_iA_iBC) {
						// A: arg window start (callee executes with base = base + A)
						// BC: function index
						Byte a = BytecodeUtil.Au(instruction);
						UInt16 funcIndex = BytecodeUtil.BCu(instruction);
						
						if (funcIndex >= functions.Count) {
							IOHelper.Print("CALLF to invalid func");
							return make_null();
						}
						
						FuncDef callee = functions[funcIndex];

						// Push return info
						if (callStackTop >= callStack.Count) {
							IOHelper.Print("Call stack overflow");
							return make_null();
						}
						callStack[callStackTop] = new CallInfo(pc, baseIndex);
						callStackTop++;

						// Switch to callee frame: base slides to argument window
						baseIndex = baseIndex + a;
						pc = 0; // Start at beginning of callee code
						entry = callee; // Switch to callee function
						
						EnsureFrame(baseIndex, callee.MaxRegs);
						break; // CPP: VM_NEXT();
					}

					case Opcode.RETURN: { // CPP: VM_CASE(RETURN) {
						// Return value convention: value is in base[0]
						if (callStackTop == 0) {
							// Returning from entry: produce the final result from base[0]
							return stack[baseIndex];
						}
						
						// Pop call stack
						callStackTop--;
						CallInfo callInfo = callStack[callStackTop];
						pc = callInfo.ReturnPC;
						baseIndex = callInfo.ReturnBase;
						
						// Need to restore the caller's function - this is a limitation of this simple approach
						// In a more complete implementation, we'd need to track the function in CallInfo
						// For now, we assume single-function execution
						break; // CPP: VM_NEXT();
					}

					// CPP: VM_DISPATCH_END();
//*** BEGIN CS_ONLY ***
					default:
						IOHelper.Print("Unknown opcode");
						return make_null();
				}
//*** END CS_ONLY ***
			}
		}

		private void EnsureFrame(Int32 baseIndex, UInt16 neededRegs) {
			// Simple implementation - just check bounds
			if (baseIndex + neededRegs > stack.Count) {
				// Simple error handling - just print and continue
				IOHelper.Print("Stack overflow error");
			}
		}
	}
}

//*** END CS_ONLY ***
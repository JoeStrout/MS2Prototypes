using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
// CPP: #include "value.h"
// CPP: #include "value_list.h"
// CPP: #include "value_string.h"
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
		public Int32 ReturnFuncIndex;  // caller's function index in functions list
		public Value LocalVarMap;      // VarMap representing locals, if any

		public CallInfo(Int32 returnPC, Int32 returnBase, Int32 returnFuncIndex) {
			ReturnPC = returnPC;
			ReturnBase = returnBase;
			ReturnFuncIndex = returnFuncIndex;
			LocalVarMap = make_null();
		}
	}

	// VM state
	public class VM {
		public Boolean DebugMode = false;
		private List<Value> stack;
		private List<Value> names;		// Variable names parallel to stack (null if unnamed)

		private List<CallInfo> callStack;
		private Int32 callStackTop;	   // Index of next free call stack slot

		private List<FuncDef> functions; // functions addressed by CALLF

		// Execution state (persistent across RunSteps calls)
		public Int32 PC { get; private set; }
		private Int32 _currentFuncIndex = -1;
		public FuncDef CurrentFunction { get; private set; }
		public Boolean IsRunning { get; private set; }
		public Int32 BaseIndex { get; private set; }
		public Int32 StackSize() {
			return stack.Count;
		}
		public Int32 CallStackDepth() {
			return callStackTop;
		}

		public Value GetStackValue(Int32 index) {
			if (index < 0 || index >= stack.Count) return make_null();
			return stack[index];
		}

		public Value GetStackName(Int32 index) {
			if (index < 0 || index >= names.Count) return make_null();
			return names[index];
		}

		public CallInfo GetCallStackFrame(Int32 index) {
			if (index < 0 || index >= callStackTop) return new CallInfo(0, 0, -1);
			return callStack[index];
		}

		public String GetFunctionName(Int32 funcIndex) {
			if (funcIndex < 0 || funcIndex >= functions.Count) return "???";
			return functions[funcIndex].Name;
		}

		public VM() {
			InitVM(1024, 256);
		}
		
		public VM(Int32 stackSlots, Int32 callSlots) {
			InitVM(stackSlots, callSlots);
		}

		private void InitVM(Int32 stackSlots, Int32 callSlots) {
			stack = new List<Value>();
			names = new List<Value>();
			callStack = new List<CallInfo>();
			functions = new List<FuncDef>();
			callStackTop = 0;

			// Initialize stack with null values
			for (Int32 i = 0; i < stackSlots; i++) {
				stack.Add(make_null());
				names.Add(make_null());		// No variable name initially
			}
			
			
			// Pre-allocate call stack capacity
			for (Int32 i = 0; i < callSlots; i++) {
				callStack.Add(new CallInfo(0, 0, -1)); // -1 = invalid function index
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
			if (DebugMode) {
				IOHelper.Print(StringUtils.Format("Executing {0} out of {1} functions", mainFunc.Name, functions.Count));
			}
			return Execute(mainFunc);
		}

		public void Reset(List<FuncDef> allFunctions) {
			// Store all functions for CALLF instructions, and find @main
			FuncDef mainFunc = null; // CPP: FuncDef mainFunc;
			functions.Clear();
			for (Int32 i = 0; i < allFunctions.Count; i++) {
				functions.Add(allFunctions[i]);
				if (functions[i].Name == "@main") mainFunc = functions[i];
			}

			if (!mainFunc) {
				IOHelper.Print("ERROR: No @main function found in VM.Reset");
				return;
			}

			// Basic validation - simplified for C++ transpilation
			if (mainFunc.Code.Count == 0) {
				IOHelper.Print("Entry function has no code");
				return;
			}

			// Find the entry function index
			_currentFuncIndex = -1;
			for (Int32 i = 0; i < functions.Count; i++) {
				if (functions[i].Name == mainFunc.Name) {
					_currentFuncIndex = i;
					break;
				}
			}

			// Initialize execution state
			BaseIndex = 0;			  // entry executes at stack base
			PC = 0;				 // start at entry code
			CurrentFunction = mainFunc;
			IsRunning = true;
			callStackTop = 0;

			EnsureFrame(BaseIndex, CurrentFunction.MaxRegs);

			if (DebugMode) {
				IOHelper.Print(StringUtils.Format("VM Reset: Executing {0} out of {1} functions", mainFunc.Name, functions.Count));
			}
		}

		public Value Execute(FuncDef entry) {
			return Execute(entry, 0);
		}

		public Value Execute(FuncDef entry, UInt32 maxCycles) {
			// Legacy method - convert to Reset + Run pattern
			List<FuncDef> singleFunc = new List<FuncDef>();
			singleFunc.Add(entry);
			Reset(singleFunc);
			return Run(maxCycles);
		}

		public Value Run(UInt32 maxCycles=0) {
			if (!IsRunning || !CurrentFunction) {
				return make_null();
			}

			// Copy instance variables to locals for performance
			Int32 pc = PC;
			Int32 baseIndex = BaseIndex;
			Int32 currentFuncIndex = _currentFuncIndex;

			FuncDef curFunc = CurrentFunction; // CPP: FuncDef& curFunc = CurrentFunction;
			Int32 codeCount = curFunc.Code.Count;
			var curCode = curFunc.Code; // CPP: UInt32* curCode = &curFunc.Code[0];
			var curConstants = curFunc.Constants; // CPP: Value* curConstants = &curFunc.Constants[0];

			UInt32 cyclesLeft = maxCycles;
			if (maxCycles == 0) cyclesLeft--;  // wraps to MAX_UINT32

/*** BEGIN CPP_ONLY ***
			Value* stackPtr = &stack[0];
#if VM_USE_COMPUTED_GOTO
			static void* const vm_labels[(int)Opcode::OP__COUNT] = { VM_OPCODES(VM_LABEL_LIST) };
			if (DebugMode) IOHelper::Print("(Running with computed-goto dispatch)");
#else
			if (DebugMode) IOHelper::Print("(Running with switch-based dispatch)");
#endif
*** END CPP_ONLY ***/

			while (IsRunning) {
				// CPP: VM_DISPATCH_TOP();
				if (cyclesLeft == 0) {
					// Update instance variables before returning
					PC = pc;
					BaseIndex = baseIndex;
					_currentFuncIndex = currentFuncIndex;
					CurrentFunction = curFunc;
					return make_null();
				}
				cyclesLeft--;

				if (pc >= codeCount) {
					IOHelper.Print("VM: PC out of bounds");
					IsRunning = false;
					// Update instance variables before returning
					PC = pc;
					BaseIndex = baseIndex;
					_currentFuncIndex = currentFuncIndex;
					CurrentFunction = curFunc;
					return make_null();
				}

				UInt32 instruction = curCode[pc++];
				Span<Value> localStack = CollectionsMarshal.AsSpan(stack).Slice(baseIndex); // CPP: Value* localStack = stackPtr + baseIndex;

				if (DebugMode) {
					// Debug output disabled for C++ transpilation
					IOHelper.Print(StringUtils.Format("{0} {1}: {2}     r0:{3}, r1:{4}, r2:{5}",
						curFunc.Name,
						StringUtils.ZeroPad(pc-1, 4),
						Disassembler.ToString(instruction),
						localStack[0], localStack[1], localStack[2]));
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
						localStack[a] = localStack[b];
						break; // CPP: VM_NEXT();
					}

					case Opcode.LOAD_rA_iBC: { // CPP: VM_CASE(LOAD_rA_iBC) {
						// R[A] = BC (signed 16-bit immediate as integer)
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						localStack[a] = make_int(bc);
						break; // CPP: VM_NEXT();
					}

					case Opcode.LOAD_rA_kBC: { // CPP: VM_CASE(LOAD_rA_kBC) {
						// R[A] = constants[BC]
						Byte a = BytecodeUtil.Au(instruction);
						UInt16 constIdx = BytecodeUtil.BCu(instruction);
						localStack[a] = curConstants[constIdx];
						break; // CPP: VM_NEXT();
					}

					case Opcode.ASSIGN_rA_rB_kC: { // CPP: VM_CASE(ASSIGN_rA_rB_kC) {
						// R[A] = R[B] and names[baseIndex + A] = constants[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						localStack[a] = localStack[b];
						names[baseIndex + a] = curConstants[c];	// OFI: keep localNames?
						break; // CPP: VM_NEXT();
					}

					case Opcode.NAME_rA_kBC: { // CPP: VM_CASE(NAME_rA_kBC) {
						// names[baseIndex + A] = constants[BC] (without changing R[A])
						Byte a = BytecodeUtil.Au(instruction);
						UInt16 constIdx = BytecodeUtil.BCu(instruction);
						names[baseIndex + a] = curConstants[constIdx];	// OFI: keep localNames?
						break; // CPP: VM_NEXT();
					}

					case Opcode.ADD_rA_rB_rC: { // CPP: VM_CASE(ADD_rA_rB_rC) {
						// R[A] = R[B] + R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						localStack[a] = value_add(localStack[b], localStack[c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.SUB_rA_rB_rC: { // CPP: VM_CASE(SUB_rA_rB_rC) {
						// R[A] = R[B] - R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						localStack[a] = value_sub(localStack[b], localStack[c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.MULT_rA_rB_rC: { // CPP: VM_CASE(MULT_rA_rB_rC) {
						// R[A] = R[B] * R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						localStack[a] = value_mult(localStack[b], localStack[c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.DIV_rA_rB_rC: { // CPP: VM_CASE(DIV_rA_rB_rC) {
						// R[A] = R[B] * R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						localStack[a] = value_div(localStack[b], localStack[c]);
						break; // CPP: VM_NEXT();
					}

                    case Opcode.MOD_rA_rB_rC: { // CPP: VM_CASE(MOD_rA_rB_rC) {
						// R[A] = R[B] % R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						localStack[a] = value_mod(localStack[b], localStack[c]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.LIST_rA_iBC: { // CPP: VM_CASE(LIST_rA_iBC) {
						// R[A] = make_list(BC)
						Byte a = BytecodeUtil.Au(instruction);
						Int16 capacity = BytecodeUtil.BCs(instruction);
						localStack[a] = make_list(capacity);
						break; // CPP: VM_NEXT();
					}

					case Opcode.MAP_rA_iBC: { // CPP: VM_CASE(MAP_rA_iBC) {
						// R[A] = make_map(BC)
						Byte a = BytecodeUtil.Au(instruction);
						Int16 capacity = BytecodeUtil.BCs(instruction);
						localStack[a] = make_map(capacity);
						break; // CPP: VM_NEXT();
					}

					case Opcode.PUSH_rA_rB: { // CPP: VM_CASE(PUSH_rA_rB) {
						// list_push(R[A], R[B])
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						list_push(localStack[a], localStack[b]);
						break; // CPP: VM_NEXT();
					}

					case Opcode.INDEX_rA_rB_rC: { // CPP: VM_CASE(INDEX_rA_rB_rC) {
						// R[A] = R[B][R[C]] (supports both lists and maps)
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						Value container = localStack[b];
						Value index = localStack[c];

						if (is_list(container)) {
							localStack[a] = list_get(container, as_int(index));
						} else if (is_map(container)) {
							localStack[a] = map_get(container, index);
						} else {
							// TODO: Add error handling for unsupported types
							localStack[a] = make_null();
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.IDXSET_rA_rB_rC: { // CPP: VM_CASE(IDXSET_rA_rB_rC) {
						// R[A][R[B]] = R[C] (supports both lists and maps)
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						Value container = localStack[a];
						Value index = localStack[b];
						Value value = localStack[c];

						if (is_list(container)) {
							list_set(container, as_int(index), value);
						} else if (is_map(container)) {
							map_set(container, index, value);
						}
						// TODO: Add error handling for unsupported types
						break; // CPP: VM_NEXT();
					}

					case Opcode.LOCALS_rA: { // CPP: VM_CASE(LOCALS_rA) {
						// Create VarMap for local variables and store in R[A]
						Byte a = BytecodeUtil.Au(instruction);
						
						CallInfo frame = callStack[callStackTop]; // CPP: CallInfo& frame = callStack[callStackTop];
						if (is_null(frame.LocalVarMap)) {
							// Create a new VarMap with references to VM's stack and names arrays
							// For now, assume our variables are within the first 5 registers.
							frame.LocalVarMap = make_varmap(stack, names, baseIndex, 5); // CPP: frame.LocalVarMap = make_varmap(&stack[0], &names[0], baseIndex, 5);
						}
						// ToDo:
						//   1. Have assembler keep track of how many registers each function
						//      needs, and use that here (instead of hard-coded 5).
						// ✅ 2. Cache the VarMap in the CallInfo, and return the cached one 
						//		rather than creating a new one every time.
						// ✅ 3. In RETURN, if the current call stack has a cached VarMap,
						//		call Gather on it.

						localStack[a] = frame.LocalVarMap;
						names[baseIndex+a] = make_null();
						break; // CPP: VM_NEXT();
					}

					case Opcode.JUMP_iABC: { // CPP: VM_CASE(JUMP_iABC) {
						// Jump by signed 24-bit ABC offset from current PC
						Int32 offset = BytecodeUtil.ABCs(instruction);
						pc += offset;
						break; // CPP: VM_NEXT();
					}

					case Opcode.LT_rA_rB_rC: { // CPP: VM_CASE(LT_rA_rB_rC) {
						// if R[A] = R[B] < R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);

						localStack[a] = make_int(value_lt(localStack[b], localStack[c]));
						break; // CPP: VM_NEXT();
					}

					case Opcode.LT_rA_rB_iC: { // CPP: VM_CASE(LT_rA_rB_iC) {
						// if R[A] = R[B] < C (immediate)
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte c = BytecodeUtil.Cs(instruction);
						
						localStack[a] = make_int(value_lt(localStack[b], make_int(c)));
						break; // CPP: VM_NEXT();
					}

					case Opcode.LT_rA_iB_rC: { // CPP: VM_CASE(LT_rA_iB_rC) {
						// if R[A] = B (immediate) < R[C]
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						
						localStack[a] = make_int(value_lt(make_int(b), localStack[c]));
						break; // CPP: VM_NEXT();
					}

					case Opcode.LE_rA_rB_rC: { // CPP: VM_CASE(LE_rA_rB_rC) {
						// if R[A] = R[B] <= R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);

						localStack[a] = make_int(value_le(localStack[b], localStack[c]));
						break; // CPP: VM_NEXT();
					}

					case Opcode.LE_rA_rB_iC: { // CPP: VM_CASE(LE_rA_rB_iC) {
						// if R[A] = R[B] <= C (immediate)
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte c = BytecodeUtil.Cs(instruction);
						
						localStack[a] = make_int(value_le(localStack[b], make_int(c)));
						break; // CPP: VM_NEXT();
					}

					case Opcode.LE_rA_iB_rC: { // CPP: VM_CASE(LE_rA_iB_rC) {
						// if R[A] = B (immediate) <= R[C]
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						Byte c = BytecodeUtil.Cu(instruction);
						
						localStack[a] = make_int(value_le(make_int(b), localStack[c]));
						break; // CPP: VM_NEXT();
					}

					case Opcode.EQ_rA_rB_rC: { // CPP: VM_CASE(EQ_rA_rB_rC) {
						// if R[A] = R[B] == R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);

						localStack[a] = make_int(value_equal(localStack[b], localStack[c]));
						break; // CPP: VM_NEXT();
					}

					case Opcode.EQ_rA_rB_iC: { // CPP: VM_CASE(EQ_rA_rB_iC) {
						// if R[A] = R[B] == C (immediate)
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte c = BytecodeUtil.Cs(instruction);
						
						localStack[a] = make_int(value_equal(localStack[b], make_int(c)));
						break; // CPP: VM_NEXT();
					}

					case Opcode.NE_rA_rB_rC: { // CPP: VM_CASE(NE_rA_rB_rC) {
						// if R[A] = R[B] != R[C]
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						Byte c = BytecodeUtil.Cu(instruction);

						localStack[a] = make_int(!value_equal(localStack[b], localStack[c]));
						break; // CPP: VM_NEXT();
					}

					case Opcode.NE_rA_rB_iC: { // CPP: VM_CASE(NE_rA_rB_iC) {
						// if R[A] = R[B] != C (immediate)
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte c = BytecodeUtil.Cs(instruction);
						
						localStack[a] = make_int(!value_equal(localStack[b], make_int(c)));
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRTRUE_rA_iBC: { // CPP: VM_CASE(BRTRUE_rA_iBC) {
						Byte a = BytecodeUtil.Au(instruction);
						Int32 offset = BytecodeUtil.BCs(instruction);
						if (is_truthy(localStack[a])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRFALSE_rA_iBC: { // CPP: VM_CASE(BRFALSE_rA_iBC) {
						Byte a = BytecodeUtil.Au(instruction);
						Int32 offset = BytecodeUtil.BCs(instruction);
						if (!is_truthy(localStack[a])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLT_rA_rB_iC: { // CPP: VM_CASE(BRLT_rA_rB_iC) {
						// if R[A] < R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_lt(localStack[a], localStack[b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLT_rA_iB_iC: { // CPP: VM_CASE(BRLT_rA_iB_iC) {
						// if R[A] < B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_lt(localStack[a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLT_iA_rB_iC: { // CPP: VM_CASE(BRLT_iA_rB_iC) {
						// if A (immediate) < R[B] then jump offset C.
						SByte a = BytecodeUtil.As(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_lt(make_int(a), localStack[b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLE_rA_rB_iC: { // CPP: VM_CASE(BRLE_rA_rB_iC) {
						// if R[A] <= R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_le(localStack[a], localStack[b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLE_rA_iB_iC: { // CPP: VM_CASE(BRLE_rA_iB_iC) {
						// if R[A] <= B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_le(localStack[a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRLE_iA_rB_iC: { // CPP: VM_CASE(BRLE_iA_rB_iC) {
						// if A (immediate) <= R[B] then jump offset C.
						SByte a = BytecodeUtil.As(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_le(make_int(a), localStack[b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BREQ_rA_rB_iC: { // CPP: VM_CASE(BREQ_rA_rB_iC) {
						// if R[A] == R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_equal(localStack[a], localStack[b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BREQ_rA_iB_iC: { // CPP: VM_CASE(BREQ_rA_iB_iC) {
						// if R[A] == B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (value_equal(localStack[a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRNE_rA_rB_iC: { // CPP: VM_CASE(BRNE_rA_rB_iC) {
						// if R[A] != R[B] then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (!value_equal(localStack[a], localStack[b])){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.BRNE_rA_iB_iC: { // CPP: VM_CASE(BRNE_rA_iB_iC) {
						// if R[A] != B (immediate) then jump offset C.
						Byte a = BytecodeUtil.Au(instruction);
						SByte b = BytecodeUtil.Bs(instruction);
						SByte offset = BytecodeUtil.Cs(instruction);
						if (!value_equal(localStack[a], make_int(b))){
							pc += offset;
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.IFLT_rA_rB: { // CPP: VM_CASE(IFLT_rA_rB) {
						// if R[A] < R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (!value_lt(localStack[a], localStack[b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.IFLT_rA_iBC: { // CPP: VM_CASE(IFLT_rA_iBC) {
						// if R[A] < BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (!value_lt(localStack[a], make_int(bc))) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFLT_iAB_rC: { // CPP: VM_CASE(IFLT_iAB_rC) {
						// if AB (immediate) < R[C] is false, skip next instruction
						short ab = BytecodeUtil.ABs(instruction);
                        Byte c = BytecodeUtil.Cu(instruction);
						if (!value_lt(make_int(ab), localStack[c])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFLE_rA_rB: { // CPP: VM_CASE(IFLE_rA_rB) {
						// if R[A] <= R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (!value_le(localStack[a], localStack[b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

					case Opcode.IFLE_rA_iBC: { // CPP: VM_CASE(IFLE_rA_iBC) {
						// if R[A] <= BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (!value_le(localStack[a], make_int(bc))) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFLE_iAB_rC: { // CPP: VM_CASE(IFLE_iAB_rC) {
						// if AB (immediate) <= R[C] is false, skip next instruction
						short ab = BytecodeUtil.ABs(instruction);
                        Byte c = BytecodeUtil.Cu(instruction);
						if (!value_le(make_int(ab), localStack[c])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFEQ_rA_rB: { // CPP: VM_CASE(IFEQ_rA_rB) {
						// if R[A] == R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (!value_equal(localStack[a], localStack[b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFEQ_rA_iBC: { // CPP: VM_CASE(IFEQ_rA_iBC) {
						// if R[A] == BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (!value_equal(localStack[a], make_int(bc))) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFNE_rA_rB: { // CPP: VM_CASE(IFNE_rA_rB) {
						// if R[A] != R[B] is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						Byte b = BytecodeUtil.Bu(instruction);
						if (value_equal(localStack[a], localStack[b])) {
							pc++; // Skip next instruction
						}
						break; // CPP: VM_NEXT();
					}

                    case Opcode.IFNE_rA_iBC: { // CPP: VM_CASE(IFNE_rA_iBC) {
						// if R[A] != BC (immediate) is false, skip next instruction
						Byte a = BytecodeUtil.Au(instruction);
						short bc = BytecodeUtil.BCs(instruction);
						if (value_equal(localStack[a], make_int(bc))) {
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
						callStack[callStackTop] = new CallInfo(pc, baseIndex, currentFuncIndex);
						callStackTop++;

						// Switch to callee frame: base slides to argument window
						baseIndex = baseIndex + a;
						pc = 0; // Start at beginning of callee code
						curFunc = callee; // Switch to callee function
						codeCount = curFunc.Code.Count;
						curCode = curFunc.Code; // CPP: curCode = &curFunc.Code[0];
						curConstants = curFunc.Constants; // CPP: curConstants = &curFunc.Constants[0];
						currentFuncIndex = funcIndex; // Switch to callee function index

						EnsureFrame(baseIndex, callee.MaxRegs);
						break; // CPP: VM_NEXT();
					}
					
					case Opcode.CALLFN_iA_kBC: { // CPP: VM_CASE(CALLFN_iA_kBC) {
						// Call named (intrinsic?) function kBC,
						// with parameters/return at register A.
						Byte a = BytecodeUtil.Au(instruction);
						UInt16 constIdx = BytecodeUtil.BCu(instruction);
						Value funcName = curConstants[constIdx];
						// For now, we'll only support intrinsics.
						// ToDo: change this once we have variable look-up.
						DoIntrinsic(funcName, baseIndex + a);
						break; // CPP: VM_NEXT();
					}

					case Opcode.RETURN: { // CPP: VM_CASE(RETURN) {
						// Return value convention: value is in base[0]
						if (callStackTop == 0) {
							// Returning from main function: update instance vars and set IsRunning = false
							PC = pc;
							BaseIndex = baseIndex;
							_currentFuncIndex = currentFuncIndex;
							CurrentFunction = curFunc;
							IsRunning = false;
							return stack[baseIndex];
						}
						
						// If current call frame had a locals VarMap, gather it up
						CallInfo frame = callStack[callStackTop]; // CPP: CallInfo& frame = callStack[callStackTop];
						if (!is_null(frame.LocalVarMap)) {
							varmap_gather(frame.LocalVarMap);
							frame.LocalVarMap = make_null();  // then clear from call frame
						}

						// Pop call stack
						callStackTop--;
						CallInfo callInfo = callStack[callStackTop];
						pc = callInfo.ReturnPC;
						baseIndex = callInfo.ReturnBase;
						currentFuncIndex = callInfo.ReturnFuncIndex; // Restore the caller's function index
						curFunc = functions[currentFuncIndex]; // Restore the caller's function
						codeCount = curFunc.Code.Count;
						curCode = curFunc.Code; // CPP: curCode = &curFunc.Code[0];
						curConstants = curFunc.Constants; // CPP: curConstants = &curFunc.Constants[0];
						break; // CPP: VM_NEXT();
					}

					// CPP: VM_DISPATCH_END();
//*** BEGIN CS_ONLY ***
					default:
						IOHelper.Print("Unknown opcode");
						// Update instance variables before returning
						PC = pc;
						BaseIndex = baseIndex;
						_currentFuncIndex = currentFuncIndex;
						CurrentFunction = curFunc;
						return make_null();
				}
//*** END CS_ONLY ***
			}

			// Update instance variables after loop exit (shouldn't normally reach here)
			PC = pc;
			BaseIndex = baseIndex;
			_currentFuncIndex = currentFuncIndex;
			CurrentFunction = curFunc;
			return make_null();
		}

		private void EnsureFrame(Int32 baseIndex, UInt16 neededRegs) {
			// Simple implementation - just check bounds
			if (baseIndex + neededRegs > stack.Count) {
				// Simple error handling - just print and continue
				IOHelper.Print("Stack overflow error");
			}
		}
		
		private static readonly Value FuncNamePrint = make_string("print");
		private static readonly Value FuncNameInput = make_string("input");
		private static readonly Value FuncNameVal = make_string("val");
		private static readonly Value FuncNameRemove = make_string("remove");
		
		private void DoIntrinsic(Value funcName, Int32 baseReg) {
			// Run the named intrinsic, with its parameters and return value
			// stored in our stack starting at baseReg.
			
			// Prototype implementation:
			
			if (value_equal(funcName, FuncNamePrint)) {
				IOHelper.Print(StringUtils.Format("{0}", stack[baseReg]));
				stack[baseReg] = make_null();
			
			} else if (value_equal(funcName, FuncNameInput)) {
				String prompt = new String("");
				if (!is_null(stack[baseReg])) {
					prompt = StringUtils.Format("{0}", stack[baseReg]);
				}
				String result = IOHelper.Input(prompt);
				stack[baseReg] = 
				  make_string(result);	// CPP: make_string(result.c_str());
			
			} else if (value_equal(funcName, FuncNameVal)) {
				stack[baseReg] = to_number(stack[baseReg]);
			
			} else if (value_equal(funcName, FuncNameRemove)) {
				// Remove index r1 from map r0; return (in r0) 1 if successful,
				// 0 if index not found.
				Value container = stack[baseReg];
				int result = 0;
				if (is_list(container)) {
					result = list_remove(container, as_int(stack[baseReg+1])) ? 1 : 0;
				} else if (is_map(container)) {
					result = map_remove(container, stack[baseReg+1]) ? 1 : 0;
				} else {
					IOHelper.Print("ERROR: `remove` must be called on list or map");
				}
				stack[baseReg] = make_int(result);
			
			} else {
				IOHelper.Print(
				  StringUtils.Format("ERROR: Unknown function '{0}'", funcName)
				);
				stack[baseReg] = make_null();
				// ToDo: put VM in an error state, so it aborts.
			}
		}
	}
}

//*** END CS_ONLY ***
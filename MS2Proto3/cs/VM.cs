using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
// CPP: #include "nanbox.h"
// CPP: #include "Bytecode.g.h"
// CPP: #include "IOHelper.g.h"

using static MiniScript.ValueHelpers;

namespace MiniScript {

    // Call stack frame (return info)
    public struct CallInfo {
        public Int32 ReturnPC;           // where to continue in caller (PC index)
        public Int32 ReturnBase;         // caller's base pointer (stack index)

        public CallInfo(Int32 returnPC, Int32 returnBase) {
            ReturnPC = returnPC;
            ReturnBase = returnBase;
        }
    }

    // VM state
    public class VM {
        private List<Value> stack;
        
        private List<CallInfo> callStack;
        private Int32 callStackTop;       // Index of next free call stack slot
        
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
            Int32 baseIndex = 0;              // entry executes at stack base
            Int32 pc = 0;                     // start at entry code
            UInt32 cycleCount = 0;
            bool debug = true;             // Set to true for debug output

            EnsureFrame(baseIndex, entry.MaxRegs);

            while (true) {
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
                    // IOHelper.Print("VM instruction: " + instruction.ToString());
                }

                Opcode opcode = (Opcode)BytecodeUtil.OP(instruction);
                
                switch (opcode) {
                    case Opcode.NOOP: {
                        // No operation
                        break;
                    }

                    case Opcode.LOAD_rA_rB: {
                        // R[A] = R[B] (equivalent to MOVE)
                        Byte a = BytecodeUtil.Au(instruction);
                        Byte b = BytecodeUtil.Bu(instruction);
                        stack[baseIndex + a] = stack[baseIndex + b];
                        break;
                    }

                    case Opcode.LOAD_rA_iBC: {
                        // R[A] = BC (signed 16-bit immediate as integer)
                        Byte a = BytecodeUtil.Au(instruction);
                        short bc = BytecodeUtil.BCs(instruction);
                        stack[baseIndex + a] = make_int(bc);
                        break;
                    }

                    case Opcode.LOAD_rA_kBC: {
                        // R[A] = constants[BC]
                        Byte a = BytecodeUtil.Au(instruction);
                        UInt16 constIdx = BytecodeUtil.BCu(instruction);
                        if (constIdx >= entry.Constants.Count) {
                            IOHelper.Print("LOAD_rA_kBC: invalid constant index");
                            return make_null();
                        }
                        stack[baseIndex + a] = entry.Constants[constIdx];
                        break;
                    }

                    case Opcode.ADD_rA_rB_rC: {
                        // R[A] = R[B] + R[C]
                        Byte a = BytecodeUtil.Au(instruction);
                        Byte b = BytecodeUtil.Bu(instruction);
                        Byte c = BytecodeUtil.Cu(instruction);
                        stack[baseIndex + a] = value_add(stack[baseIndex + b], stack[baseIndex + c]);
                        break;
                    }

                    case Opcode.SUB_rA_rB_rC: {
                        // R[A] = R[B] - R[C]
                        Byte a = BytecodeUtil.Au(instruction);
                        Byte b = BytecodeUtil.Bu(instruction);
                        Byte c = BytecodeUtil.Cu(instruction);
                        stack[baseIndex + a] = value_sub(stack[baseIndex + b], stack[baseIndex + c]);
                        break;
                    }

                    case Opcode.JUMP_iABC: {
                        // Jump by signed 24-bit ABC offset from current PC
                        Int32 offset = BytecodeUtil.ABCs(instruction);
                        pc += offset;
                        break;
                    }

                    case Opcode.IFLT_rA_rB: {
                        // if R[A] < R[B] is false, skip next instruction
                        Byte a = BytecodeUtil.Au(instruction);
                        Byte b = BytecodeUtil.Bu(instruction);
                        if (!value_lt(stack[baseIndex + a], stack[baseIndex + b])) {
                            pc++; // Skip next instruction
                        }
                        break;
                    }

                    case Opcode.IFLT_rA_iBC: {
                        // if R[A] < BC (immediate) is false, skip next instruction
                        Byte a = BytecodeUtil.Au(instruction);
                        short bc = BytecodeUtil.BCs(instruction);
                        if (!value_lt(stack[baseIndex + a], make_int(bc))) {
                            pc++; // Skip next instruction
                        }
                        break;
                    }

                    case Opcode.CALLF_iA_iBC: {
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
                        break;
                    }

                    case Opcode.RETURN: {
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
                        break;
                    }

                    default:
                        IOHelper.Print("Unknown opcode");
                        return make_null();
                }
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
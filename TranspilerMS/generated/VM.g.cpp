#include "VM.g.h"

namespace ScriptingVM {
    // Opcodes matching the C VM implementation

    // Instruction field extraction helpers (matching C implementation)

        // Instruction encoding helpers

    // Function prototype (equivalent to C Proto struct)


        Proto::Proto(List<UInt32> code, UInt16 maxRegs, List<Value> constants) {
            Code = code;
            MaxRegs = maxRegs;
            Constants = constants;
            if (!Code) Code = List<UInt32>();
            if (!Constants) Constants = List<Value>();
        }

        Proto::Proto() {
            Code = List<UInt32>();
        	MaxRegs = 0; // frame reservation size
        	Constants = List<Value>();
        }
	
    // Call stack frame (return info) - equivalent to C CallInfo struct

        CallInfo::CallInfo(Int32 returnPC, Int32 returnBase) {
            ReturnPC = returnPC;
            ReturnBase = returnBase;
        }

    // VM state - equivalent to C VM struct
        
        

        VM::VM(Int32 stackSlots, Int32 callSlots) {
            _stack = List<Value>(stackSlots);
            _stackSize = stackSlots;
            _top = 0;
            
            _callStack = List<CallInfo>(callSlots);
            _callStackSize = callSlots;
            _callIndex = 0;
            
            // Initialize all stack values to null
            for (Int32 i = 0; i < stackSlots; i++) {
                _stack.Add(Value.Null());
            }
            
            // Initialize functions list with nulls
            for (Int32 i = 0; i < 256; i++) {
                _functions.Add(null);
            }
        }

        // Register a function for CALLF calls
        void VM::RegisterFunction(Byte index, Proto function) {
            if (index < _functions.Count) {
                _functions[index] = function;
            } else {
            	// ToDo: handle error
            }
        }

        // Execute a function prototype
        Value VM::Execute(Proto entry, UInt32 maxCycles = 0) {
            // Current frame state (equivalent to C locals)
            Int32 baseReg = 0; // entry executes at stack base (index 0)
            Int32 pc = 0; // program counter (index into entry.Code)
            
            EnsureFrame(baseReg, entry.MaxRegs);
            
            UInt32 cycleCount = 0;
            bool debug = false; // Set to true for debug output

            while (true) {
                cycleCount++;
                if (maxCycles > 0 && cycleCount > maxCycles) {
                    IOHelper.Print("VM: Hit cycle limit of " + str(maxCycles));
                    return Value.Null();
                }

                if (pc >= entry.Code.Count) {
                    IOHelper.Print("VM: PC out of bounds");
                    return Value.Null();
                }

                UInt32 instruction = entry.Code[pc++];
                
                if (debug) {
                    IOHelper.Print($"PC: {pc - 1}, Cycle: {cycleCount}, Ins: 0x{instruction:X8}, Op: {InstructionHelpers::OP(instruction)}");
                    // ToDo: make C++ equivalent of this $ string interpolation thing
                }

                Opcode op = (Opcode)InstructionHelpers::OP(instruction);
                
                switch (op) {
                    case Opcode::MOVE:
                        // R[A] = R[B]
                        _stack[baseReg + InstructionHelpers::A(instruction)] = 
                            _stack[baseReg + InstructionHelpers::B(instruction)];
                        break;

                    case Opcode::LOADK:
                        // R[A] = BC (16-bit signed immediate)
                        _stack[baseReg + InstructionHelpers::A(instruction)] = 
                            Value.FromInt(InstructionHelpers::BC(instruction));
                        break;

                    case Opcode::LOADN: {
                            // R[A] = constants[BC]
                        
                            UInt16 constIdx = (UInt16)InstructionHelpers::BC(instruction);
                            if (constIdx >= entry.ConstLength) {
                                Console.Error.WriteLine($"LOADN: invalid constant index {constIdx}");
                                return Value.Null();
                            }
                            _stack[baseReg + InstructionHelpers::A(instruction)] = entry.Constants[constIdx];
                        }
                        break;

                    case Opcode::ADD:
                        // R[A] = R[B] + R[C]
                        _stack[baseReg + InstructionHelpers::A(instruction)] = 
                            Value.Add(_stack[baseReg + InstructionHelpers::B(instruction)],
                                     _stack[baseReg + InstructionHelpers::C(instruction)]);
                        break;

                    case Opcode::SUB:
                        // R[A] = R[B] - R[C]
                        _stack[baseReg + InstructionHelpers::A(instruction)] = 
                            Value.Sub(_stack[baseReg + InstructionHelpers::B(instruction)],
                                     _stack[baseReg + InstructionHelpers::C(instruction)]);
                        break;

                    case Opcode::IFLT: {
                            // if R[A] < R[B], jump by signed 8-bit C from current pc
                            sByte offset = (sByte)InstructionHelpers::C(instruction);
                            if (Value.LessThan(_stack[baseReg + InstructionHelpers::A(instruction)],
                                              _stack[baseReg + InstructionHelpers::B(instruction)])) {
                                pc += offset;
                            }
                        }
                        break;

                    case Opcode::JMP:
                        // PC += BC (16-bit signed offset)
                        pc += InstructionHelpers::BC(instruction);
                        break;

                    case Opcode::CALLF: {
							// A: arg window start (callee executes with base = base + A)
							// B: nargs (ignored in this minimal VM, but here for shape)  
							// C: function index (0..255)
                            Byte funcIdx = InstructionHelpers::C(instruction);
                            Proto callee = _functions[funcIdx];
                            if (callee == null) {
                                Console.Error.WriteLine($"CALLF to null func {funcIdx}");
                                return Value.Null();
                            }

                            // Push return info
                            if (_callIndex >= _callStackSize) {
                                Console.Error.WriteLine("call stack overflow");
                                return Value.Null();
                            }
                            // Ensure call stack has enough capacity
                            while (_callStack.Count <= _callIndex) {
                                _callStack.Add(new CallInfo());
                            }
                            _callStack[_callIndex] = CallInfo(pc, baseReg);
                            _callIndex++;

                            // Switch to callee frame: base slides to argument window
                            baseReg = baseReg + InstructionHelpers::A(instruction);
                            pc = 0; // start at beginning of callee code
                            entry = callee; // switch to callee's code and constants

                            EnsureFrame(baseReg, callee.MaxRegs);
                        }
                        break;

                    case Opcode::RETURN:
                        // Return value convention: value is in R[A] (relative to current base)
                        // Since callee base points into caller's frame (at arg window),
                        // the result is already in-place for the caller. We just pop.
                        if (_callIndex == 0) {
                            // Returning from entry function: produce the final result from R[0]
                            return _stack[baseReg];
                        }
                        // Pop call frame
                        _callIndex--;
                        CallInfo returnInfo = _callStack[_callIndex];
                        pc = returnInfo.ReturnPC;
                        baseReg = returnInfo.ReturnBase;
                        
                        // We need to restore the caller's Proto, but in this simplified version
                        // we'll assume we're only calling functions in the same Proto for now.
                        // A more complete implementation would need to track the caller's Proto.
                        break;

                    default:
                        IOHelper.Print("Unknown opcode: " + str(op));
                        return Value.Null();
                }
            }
        }

        void VM::EnsureFrame(Int32 baseReg, UInt16 needRegs) {
            // In this demo, stack is pre-allocated large enough, so this is a no-op
            // A real implementation might need to grow the stack here
            Int32 requiredSize = baseReg + needRegs;
            if (requiredSize > _stackSize) {
                IOHelper.Print($"Stack overflow: need {requiredSize}, have {_stackSize}");
            }
        }

        // Debug helper to print stack state
        void VM::PrintStack(Int32 baseReg, Int32 count) {
            IOHelper.Print("Stack:");
            for (Int32 i = 0; i < count; i++) {
                Value val = _stack[baseReg + i];
                IOHelper.Print($"  R[{i}] = {val}");
            }
        }


}

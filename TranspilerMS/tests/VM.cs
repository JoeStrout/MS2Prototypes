using System;
using System.Collections.Generic;

namespace ScriptingVM {
    // Opcodes matching the C VM implementation
    public enum Opcode : Byte {
        MOVE = 0,
        LOADK = 1,
        LOADN = 2,
        ADD = 3,
        SUB = 4,
        IFLT = 5,
        JMP = 6,
        CALLF = 7,
        RETURN = 8
    }

    // Instruction field extraction helpers (matching C implementation)
    public static class InstructionHelpers {
        public static Byte OP(UInt32 instruction) => (Byte)((instruction >> 24) & 0xFF);
        public static Byte A(UInt32 instruction) => (Byte)((instruction >> 16) & 0xFF);
        public static Byte B(UInt32 instruction) => (Byte)((instruction >> 8) & 0xFF);
        public static Byte C(UInt32 instruction) => (Byte)(instruction & 0xFF);
        public static Int16 BC(UInt32 instruction) => (Int16)(instruction & 0xFFFF);

        // Instruction encoding helpers
        public static UInt32 INS(Opcode op) => (UInt32)((Byte)op << 24);
        public static UInt32 INS_ABC(Opcode op, Byte a, Byte b, Byte c) => (UInt32)(((Byte)op << 24) | (a << 16) | (b << 8) | c);
        public static UInt32 INS_AB(Opcode op, Byte a, Int16 bc) => (UInt32)(((Byte)op << 24) | (a << 16) | ((UInt16)bc));
    }

    // Function prototype (equivalent to C Proto struct)
    public class Proto {
        public List<UInt32> Code = new List<UInt32>();
        public UInt16 VarRegs; // frame reservation size
        public List<Value> Constants = new List<Value>();

        public Int32 CodeLength => Code.Count;
        public Int32 ConstLength => Constants.Count;

        public Proto(List<UInt32> code, UInt16 maxRegs, List<Value> constants) {
            Code = code;
            VarRegs = maxRegs;
            Constants = constants;
            if (!Code) Code = new List<UInt32>();
            if (!Constants) Constants = new List<Value>();
        }

        public Proto() {
            Code = new List<UInt32>();
        	VarRegs = 0; // frame reservation size
        	Constants = new List<Value>();
        }
    }
	
    // Call stack frame (return info) - equivalent to C CallInfo struct
    public struct CallInfo {
        public Int32 ReturnPC;     // index into caller's code array (not pointer)
        public Int32 ReturnBase;   // caller's base register index

        public CallInfo(Int32 returnPC, Int32 returnBase) {
            ReturnPC = returnPC;
            ReturnBase = returnBase;
        }
    }

    // VM state - equivalent to C VM struct
    public class VM {
        private List<Value> _stack;
        private Int32 _stackSize;
        private Int32 _top; // index into stack (not pointer)
        
        private List<CallInfo> _callStack;
        private Int32 _callStackSize;
        private Int32 _callIndex; // points to next free slot
        
        private List<Proto> _functions = new List<Proto>(256); // functions addressed by CALLF C-field

        public VM(Int32 stackSlots, Int32 callSlots) {
            _stack = new List<Value>(stackSlots);
            _stackSize = stackSlots;
            _top = 0;
            
            _callStack = new List<CallInfo>(callSlots);
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
        public void RegisterFunction(Byte index, Proto function) {
            if (index < _functions.Count) {
                _functions[index] = function;
            } else {
            	// ToDo: handle error
            }
        }

        // Execute a function prototype
        public Value Execute(Proto entry, UInt32 maxCycles = 0) {
            // Current frame state (equivalent to C locals)
            Int32 baseReg = 0; // entry executes at stack base (index 0)
            Int32 pc = 0; // program counter (index into entry.Code)
            
            EnsureFrame(baseReg, entry.VarRegs);
            
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
                    IOHelper.Print($"PC: {pc - 1}, Cycle: {cycleCount}, Ins: 0x{instruction:X8}, Op: {InstructionHelpers.OP(instruction)}");
                    // ToDo: make C++ equivalent of this $ string interpolation thing
                }

                Opcode op = (Opcode)InstructionHelpers.OP(instruction);
                
                switch (op) {
                    case Opcode.MOVE:
                        // R[A] = R[B]
                        _stack[baseReg + InstructionHelpers.A(instruction)] = 
                            _stack[baseReg + InstructionHelpers.B(instruction)];
                        break;

                    case Opcode.LOADK:
                        // R[A] = BC (16-bit signed immediate)
                        _stack[baseReg + InstructionHelpers.A(instruction)] = 
                            Value.FromInt(InstructionHelpers.BC(instruction));
                        break;

                    case Opcode.LOADN: {
                            // R[A] = constants[BC]
                        
                            UInt16 constIdx = (UInt16)InstructionHelpers.BC(instruction);
                            if (constIdx >= entry.ConstLength) {
                                Console.Error.WriteLine($"LOADN: invalid constant index {constIdx}");
                                return Value.Null();
                            }
                            _stack[baseReg + InstructionHelpers.A(instruction)] = entry.Constants[constIdx];
                        }
                        break;

                    case Opcode.ADD:
                        // R[A] = R[B] + R[C]
                        _stack[baseReg + InstructionHelpers.A(instruction)] = 
                            Value.Add(_stack[baseReg + InstructionHelpers.B(instruction)],
                                     _stack[baseReg + InstructionHelpers.C(instruction)]);
                        break;

                    case Opcode.SUB:
                        // R[A] = R[B] - R[C]
                        _stack[baseReg + InstructionHelpers.A(instruction)] = 
                            Value.Sub(_stack[baseReg + InstructionHelpers.B(instruction)],
                                     _stack[baseReg + InstructionHelpers.C(instruction)]);
                        break;

                    case Opcode.IFLT: {
                            // if R[A] < R[B], jump by signed 8-bit C from current pc
                            sByte offset = (sByte)InstructionHelpers.C(instruction);
                            if (Value.LessThan(_stack[baseReg + InstructionHelpers.A(instruction)],
                                              _stack[baseReg + InstructionHelpers.B(instruction)])) {
                                pc += offset;
                            }
                        }
                        break;

                    case Opcode.JMP:
                        // PC += BC (16-bit signed offset)
                        pc += InstructionHelpers.BC(instruction);
                        break;

                    case Opcode.CALLF: {
							// A: arg window start (callee executes with base = base + A)
							// B: nargs (ignored in this minimal VM, but here for shape)  
							// C: function index (0..255)
                            Byte funcIdx = InstructionHelpers.C(instruction);
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
                            _callStack[_callIndex] = new CallInfo(pc, baseReg);
                            _callIndex++;

                            // Switch to callee frame: base slides to argument window
                            baseReg = baseReg + InstructionHelpers.A(instruction);
                            pc = 0; // start at beginning of callee code
                            entry = callee; // switch to callee's code and constants

                            EnsureFrame(baseReg, callee.VarRegs);
                        }
                        break;

                    case Opcode.RETURN:
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

        private void EnsureFrame(Int32 baseReg, UInt16 needRegs) {
            // In this demo, stack is pre-allocated large enough, so this is a no-op
            // A real implementation might need to grow the stack here
            Int32 requiredSize = baseReg + needRegs;
            if (requiredSize > _stackSize) {
                IOHelper.Print($"Stack overflow: need {requiredSize}, have {_stackSize}");
            }
        }

        // Debug helper to print stack state
        public void PrintStack(Int32 baseReg, Int32 count) {
            IOHelper.Print("Stack:");
            for (Int32 i = 0; i < count; i++) {
                Value val = _stack[baseReg + i];
                IOHelper.Print($"  R[{i}] = {val}");
            }
        }
    }

	//*** BEGIN CS_ONLY ***


    // Helper class for building Proto objects
    public class ProtoBuilder {
        private List<UInt32> _code = new List<UInt32>();
        private List<Value> _constants = new List<Value>();
        private UInt16 _maxRegs = 0;

        public ProtoBuilder SetMaxRegs(UInt16 maxRegs) {
            _maxRegs = maxRegs;
            return this;
        }
        
        public void Reset() {
        	_code = new List<UInt32>();
        	_constants = new List<Value>();
        	_maxRegs = 0;
        	return this;
        }

        public ProtoBuilder AddInstruction(UInt32 instruction) {
            _code.Add(instruction);
            return this;
        }

        public ProtoBuilder Move(Byte dest, Byte src) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.MOVE, dest, src, 0));
            return this;
        }

        public ProtoBuilder LoadK(Byte dest, Int16 value) {
            _code.Add(InstructionHelpers.INS_AB(Opcode.LOADK, dest, value));
            return this;
        }

        public ProtoBuilder LoadN(Byte dest, UInt16 constIndex) {
            _code.Add(InstructionHelpers.INS_AB(Opcode.LOADN, dest, (Int16)constIndex));
            return this;
        }

        public ProtoBuilder Add(Byte dest, Byte op1, Byte op2) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.ADD, dest, op1, op2));
            return this;
        }

        public ProtoBuilder Sub(Byte dest, Byte op1, Byte op2) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.SUB, dest, op1, op2));
            return this;
        }

        public ProtoBuilder IfLt(Byte a, Byte b, sByte offset) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.IFLT, a, b, (Byte)offset));
            return this;
        }

        public ProtoBuilder Jmp(Int16 offset) {
            _code.Add(InstructionHelpers.INS_AB(Opcode.JMP, 0, offset));
            return this;
        }

        public ProtoBuilder CallF(Byte argBase, Byte numArgs, Byte funcIndex) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.CALLF, argBase, numArgs, funcIndex));
            return this;
        }

        public ProtoBuilder Return() {
            _code.Add(InstructionHelpers.INS(Opcode.RETURN));
            return this;
        }

        public Int32 AddConstant(Value value) {
            Int32 index = _constants.Count;
            _constants.Add(value);
            return index;
        }

        public Proto Build() {
            return new Proto(_code, _maxRegs, _constants);
            Reset();
        }
    }
	//*** END CS_ONLY ***

}

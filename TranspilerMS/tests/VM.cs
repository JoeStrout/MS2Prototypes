using System;
using System.Collections.Generic;

namespace ScriptingVM {
    // Opcodes matching the C VM implementation
    public enum Opcode : byte {
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
        public static byte OP(uint instruction) => (byte)((instruction >> 24) & 0xFF);
        public static byte A(uint instruction) => (byte)((instruction >> 16) & 0xFF);
        public static byte B(uint instruction) => (byte)((instruction >> 8) & 0xFF);
        public static byte C(uint instruction) => (byte)(instruction & 0xFF);
        public static short BC(uint instruction) => (short)(instruction & 0xFFFF);

        // Instruction encoding helpers
        public static uint INS(Opcode op) => (uint)((byte)op << 24);
        public static uint INS_ABC(Opcode op, byte a, byte b, byte c) => (uint)(((byte)op << 24) | (a << 16) | (b << 8) | c);
        public static uint INS_AB(Opcode op, byte a, short bc) => (uint)(((byte)op << 24) | (a << 16) | ((ushort)bc));
    }

    // Function prototype (equivalent to C Proto struct)
    public class Proto {
        public List<uint> Code = new List<uint>();
        public ushort MaxRegs; // frame reservation size
        public List<Value> Constants = new List<Value>();

        public int CodeLength => Code.Count;
        public int ConstLength => Constants.Count;

        public Proto(List<uint> code, ushort maxRegs, List<Value> constants) {
            Code = code;
            MaxRegs = maxRegs;
            Constants = constants;
            if (!Code) Code = new List<uint>();
            if (!Constants) Constants = new List<Value>();
        }

        public Proto() {
            Code = new List<uint>();
        	MaxRegs = 0; // frame reservation size
        	Constants = new List<Value>();
        }
    }
	//*** BEGIN CS_ONLY ***
	
    // Call stack frame (return info) - equivalent to C CallInfo struct
    public struct CallInfo {
        public int ReturnPC;     // index into caller's code array (not pointer)
        public int ReturnBase;   // caller's base register index

        public CallInfo(int returnPC, int returnBase) {
            ReturnPC = returnPC;
            ReturnBase = returnBase;
        }
    }

    // VM state - equivalent to C VM struct
    public class VM {
        private List<Value> _stack;
        private int _stackSize;
        private int _top; // index into stack (not pointer)
        
        private List<CallInfo> _callStack;
        private int _callStackSize;
        private int _callIndex; // points to next free slot
        
        private List<Proto?> _functions = new List<Proto?>(256); // functions addressed by CALLF C-field

        public VM(int stackSlots, int callSlots) {
            _stack = new List<Value>(stackSlots);
            _stackSize = stackSlots;
            _top = 0;
            
            _callStack = new List<CallInfo>(callSlots);
            _callStackSize = callSlots;
            _callIndex = 0;
            
            // Initialize all stack values to null
            for (int i = 0; i < stackSlots; i++) {
                _stack.Add(Value.Null());
            }
            
            // Initialize functions list with nulls
            for (int i = 0; i < 256; i++) {
                _functions.Add(null);
            }
        }

        // Register a function for CALLF calls
        public void RegisterFunction(byte index, Proto function) {
            if (index < _functions.Count) {
                _functions[index] = function;
            } else {
            	// ToDo: handle error
            }
        }

        // Execute a function prototype
        public Value Execute(Proto entry, uint maxCycles = 0) {
            // Current frame state (equivalent to C locals)
            int baseReg = 0; // entry executes at stack base (index 0)
            int pc = 0; // program counter (index into entry.Code)
            
            EnsureFrame(baseReg, entry.MaxRegs);
            
            uint cycleCount = 0;
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

                uint instruction = entry.Code[pc++];
                
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
                        
                            ushort constIdx = (ushort)InstructionHelpers.BC(instruction);
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
                            sbyte offset = (sbyte)InstructionHelpers.C(instruction);
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
                            byte funcIdx = InstructionHelpers.C(instruction);
                            Proto? callee = _functions[funcIdx];
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

                            EnsureFrame(baseReg, callee.MaxRegs);
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

        private void EnsureFrame(int baseReg, ushort needRegs) {
            // In this demo, stack is pre-allocated large enough, so this is a no-op
            // A real implementation might need to grow the stack here
            int requiredSize = baseReg + needRegs;
            if (requiredSize > _stackSize) {
                IOHelper.Print($"Stack overflow: need {requiredSize}, have {_stackSize}");
            }
        }

        // Debug helper to print stack state
        public void PrintStack(int baseReg, int count) {
            IOHelper.Print("Stack:");
            for (int i = 0; i < count; i++) {
                Value val = _stack[baseReg + i];
                IOHelper.Print($"  R[{i}] = {val}");
            }
        }
    }

    // Helper class for building Proto objects
    public class ProtoBuilder {
        private List<uint> _code = new List<uint>();
        private List<Value> _constants = new List<Value>();
        private ushort _maxRegs = 0;

        public ProtoBuilder SetMaxRegs(ushort maxRegs) {
            _maxRegs = maxRegs;
            return this;
        }
        
        public void Reset() {
        	_code = new List<uint>();
        	_constants = new List<Value>();
        	_maxRegs = 0;
        	return this;
        }

        public ProtoBuilder AddInstruction(uint instruction) {
            _code.Add(instruction);
            return this;
        }

        public ProtoBuilder Move(byte dest, byte src) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.MOVE, dest, src, 0));
            return this;
        }

        public ProtoBuilder LoadK(byte dest, short value) {
            _code.Add(InstructionHelpers.INS_AB(Opcode.LOADK, dest, value));
            return this;
        }

        public ProtoBuilder LoadN(byte dest, ushort constIndex) {
            _code.Add(InstructionHelpers.INS_AB(Opcode.LOADN, dest, (short)constIndex));
            return this;
        }

        public ProtoBuilder Add(byte dest, byte op1, byte op2) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.ADD, dest, op1, op2));
            return this;
        }

        public ProtoBuilder Sub(byte dest, byte op1, byte op2) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.SUB, dest, op1, op2));
            return this;
        }

        public ProtoBuilder IfLt(byte a, byte b, sbyte offset) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.IFLT, a, b, (byte)offset));
            return this;
        }

        public ProtoBuilder Jmp(short offset) {
            _code.Add(InstructionHelpers.INS_AB(Opcode.JMP, 0, offset));
            return this;
        }

        public ProtoBuilder CallF(byte argBase, byte numArgs, byte funcIndex) {
            _code.Add(InstructionHelpers.INS_ABC(Opcode.CALLF, argBase, numArgs, funcIndex));
            return this;
        }

        public ProtoBuilder Return() {
            _code.Add(InstructionHelpers.INS(Opcode.RETURN));
            return this;
        }

        public int AddConstant(Value value) {
            int index = _constants.Count;
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
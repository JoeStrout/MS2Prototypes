using System;
using System.Collections.Generic;
using Types;

namespace VM {

    public enum OpCode
    {
        LOADK,      // Load constant: R(A) = K(Bx)
        LOADNULL,   // Load nil: R(A) through R(A+B) = null
        MOVE,       // Assign: R(A) = R(B)
        ADD,        // R(A) = RK(B) + RK(C)
        SUB,        // R(A) = RK(B) - RK(C)
        EQ,         // skip next instruction if (RK(B) == RK(C)) equals R(A)
        LT,         // skip next instruction if (RK(B) < RK(C)) equals R(A)
        JMP,        // pc += B (instead of usual pc++); A currently unused
        CALL,       // push call frame with B registers, then jump to A
        RETURN,     // R(0) = RK(B); pop top call frame
    }

    public class Instruction
    {
        public OpCode opcode;
        public short A;
        public short B;
        public short C;

        public Instruction(OpCode op, short a=0, short b=0, short c=0)
        {
            opcode = op;
            A = a;
            B = b; 
            C = c;
        }

        public override string ToString()
        {
            return $"{opcode} {A} {B} {C}";
        }
    }

    public class CallFrame
    {
        public int callLoc;         // index of call instruction in VM instructions list
        public int baseRegister;    // true index of this function's R0
        public int qtyRegisters;    // how many registers were reserved for this function

        public CallFrame(int callLoc=0, int baseRegister=0, int qtyRegisters=0) {
            this.callLoc = callLoc;
            this.baseRegister = baseRegister;
            this.qtyRegisters = qtyRegisters;
        }
    }

    public class VM
    {
        public List<Instruction> instructions = new List<Instruction>();
        public List<Value> constants = new List<Value>();
        public List<Value> registers = new List<Value>();
        public List<CallFrame> callStack = new List<CallFrame>();
        public int pc = 0;
        public int curR0 = 0;
        
        public const int regLimit = 250;   // starting here, RK operands access constants instead of registers

        public VM() {
            callStack.Add(new CallFrame(-1, 0, 10));
        }

        public void AddInstruction(Instruction instruction) {
            instructions.Add(instruction);
        }

        void PrintTwoColumns(string col1, string col2) {
            const string spaces = "                                        ";  // width of col1
            int w = spaces.Length;
            if (col1.Length > 40) col1 = col1.Substring(0, w-1) + "…";
            else col1 = col1 + spaces.Substring(0, w - col1.Length);
            Console.WriteLine(col1 + col2);
        }

        public void PrintState(int contextLines = 10) {
            Console.WriteLine("--- VM State ---");
            int firstLine = Math.Max(0, pc - contextLines / 2);
            int lastLine = Math.Min(instructions.Count - 1, pc + contextLines / 2);

            int i;
            for (i = firstLine; i <= lastLine; i++) {
                Console.WriteLine((i == pc ? "-->" : "   ") + $" {i}: {instructions[i]}");
            }
            PrintTwoColumns($"Registers ({registers.Count}):", $"Constants ({constants.Count}):");
            i = 0;
            while (true) {
                if (i >= registers.Count && i >= constants.Count) break;
                string r = "";
                if (i < registers.Count) {
                    r = (i >= curR0 ? $"  R{i - curR0}" : "    ");
                    r += $": {registers[i]}";
                }
                string k = (i < constants.Count ? $"  K{i}: {constants[i]}": "");
                PrintTwoColumns(r, k);
                i++;
            }
        }

        public bool IsDone() {
            return pc >= instructions.Count;
        }

        public void Step() {
            if (pc >= instructions.Count) {
                return;
            }
            var instruction = instructions[pc];
            switch (instruction.opcode) {
                case OpCode.LOADNULL:
                    for (int i = 0; i < instruction.B; i++) {
                        registers[curR0 + instruction.A + i] = Value.Null;
                    }
                    pc++;
                    break;
                case OpCode.LOADK:
                    registers[curR0 + instruction.A] = constants[instruction.B];
                    pc++;
                    break;
                case OpCode.MOVE:
                    registers[curR0 + instruction.A] = registers[curR0 + instruction.B];
                    pc++;
                    break;
                case OpCode.ADD:
                    {
                        Value b = instruction.B < regLimit ? registers[curR0 + instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[curR0 + instruction.C] : constants[instruction.C - regLimit];
                        registers[curR0 + instruction.A] = b + c;
                        pc++;
                    } break;
                case OpCode.SUB:
                    {
                        Value b = instruction.B < regLimit ? registers[curR0 + instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[curR0 + instruction.C] : constants[instruction.C - regLimit];
                        registers[curR0 + instruction.A] = b - c;
                        pc++;
                    } break;
                case OpCode.EQ:
                    {
                        Value b = instruction.B < regLimit ? registers[curR0 + instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[curR0 + instruction.C] : constants[instruction.C - regLimit];
                        if ((b == c ? 1 : 0) != instruction.A) pc++;
                        // Note: The Lua VM actually assumes the next instruction is a JMP, and handles that
                        // case immediately (in the same machine cycle).  We're not doing that (yet).
                        pc++;
                    }
                    break;
                case OpCode.LT:
                    {
                        Value b = instruction.B < regLimit ? registers[curR0 + instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[curR0 + instruction.C] : constants[instruction.C - regLimit];
                        if ((b < c ? 1 : 0) != instruction.A) pc++;
                        // Note: The Lua VM actually assumes the next instruction is a JMP, and handles that
                        // case immediately (in the same machine cycle).  We're not doing that (yet).
                        pc++;
                    }
                    break;
                case OpCode.JMP:
                    // Note: Lua uses opcode A to control closing upvalues.
                    // That's not a thing for us (yet).
                    pc += instruction.B;
                    break;
                case OpCode.CALL:
                    // In this prototype, call works as follows:
                    // 1. Parameters are pushed into the next unused registers beyond
                    //      the current call frame's register window
                    // 2. Operand B specifies how many registers the called function needs
                    //      (including parameters)
                    // 3. Result is returned in the first of these (called function's R0)
                    // Note that this requires a "base" call frame, so we can know how many
                    // registers the global code needs.
                    {
                        CallFrame curFrame = callStack[callStack.Count-1];
                        int newR0 = curFrame.baseRegister + curFrame.qtyRegisters;
                        callStack.Add(new CallFrame(pc, newR0, instruction.B));
                        pc = instruction.A;
                        curR0 = newR0;
                        Console.WriteLine($"Pushed call to instruction {pc} with {instruction.B} registers starting at {curR0}");
                        // Make sure we have enough registers in the VM for the new call frame, including
                        // any it might need to push arguments.  HACK: we're only allowing 1 extra for
                        // arguments.  A full implementation would probably need to keep track of how much
                        // space a function needs for arguments, in addition to its own qtyRegisters.
                        while (registers.Count < curR0 + instruction.B + 1) {
                            registers.Add(Value.Null);
                        }
                    }
                    break;
                case OpCode.RETURN:
                    if (callStack.Count <= 1) {
                        // Return from last call frame: terminate program
                        pc = instructions.Count;
                        Console.WriteLine("Return in base call frame; terminating program");
                    } else {
                        Value b = instruction.B < regLimit ? registers[curR0 + instruction.B] : constants[instruction.B - regLimit];
                        registers[curR0] = b;
                        int frameNum = callStack.Count - 1;
                        pc = callStack[frameNum].callLoc + 1;
                        callStack.RemoveAt(frameNum);
                        curR0 = callStack[frameNum-1].baseRegister;
                        Console.WriteLine($"Popped call frame {frameNum}; back to {pc} with registers starting at {curR0}");
                    }
                    break;
                default:
                    throw new Exception("Unknown instruction: " + instruction.opcode);
            }
        }
    }
}

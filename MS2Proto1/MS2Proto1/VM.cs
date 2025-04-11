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
        JMP         // pc += B (instead of usual pc++); A currently unused
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

    public class VM
    {
        public List<Instruction> instructions = new List<Instruction>();
        public List<Value> constants = new List<Value>();
        public List<Value> registers = new List<Value>();
        public int pc = 0;
        
        public const int regLimit = 250;   // starting here, RK operands access constants instead of registers

        public void AddInstruction(Instruction instruction)
        {
            instructions.Add(instruction);
        }

        public void PrintState(int contextLines = 10)
        {
            Console.WriteLine("--- VM State ---");
            int firstLine = Math.Max(0, pc - contextLines / 2);
            int lastLine = Math.Min(instructions.Count - 1, pc + contextLines / 2);

            for (int i = firstLine; i <= lastLine; i++) {
                Console.WriteLine((i == pc ? "-->" : "   ") + $" {i}: {instructions[i]}");
            }
            Console.WriteLine($"Registers ({registers.Count}): ");
            for (int i = 0; i < registers.Count; i++) {
                Console.WriteLine($"  R{i}: {registers[i]}");
            }
            Console.WriteLine($"Constants ({constants.Count}): ");
            for (int i = 0; i < constants.Count; i++) { 
                Console.WriteLine($"  K{i}: {constants[i]}");
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
                        registers[instruction.A + i] = Value.Null;
                    }
                    pc++;
                    break;
                case OpCode.LOADK:
                    registers[instruction.A] = constants[instruction.B];
                    pc++;
                    break;
                case OpCode.MOVE:
                    registers[instruction.A] = registers[instruction.B];
                    pc++;
                    break;
                case OpCode.ADD:
                    {
                        Value b = instruction.B < regLimit ? registers[instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[instruction.C] : constants[instruction.C - regLimit];
                        registers[instruction.A] = b + c;
                        pc++;
                    } break;
                case OpCode.SUB:
                    {
                        Value b = instruction.B < regLimit ? registers[instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[instruction.C] : constants[instruction.C - regLimit];
                        registers[instruction.A] = b - c;
                        pc++;
                    } break;
                case OpCode.EQ:
                    {
                        Value b = instruction.B < regLimit ? registers[instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[instruction.C] : constants[instruction.C - regLimit];
                        if ((b == c ? 1 : 0) != instruction.A) pc++;
                        // Note: The Lua VM actually assumes the next instruction is a JMP, and handles that
                        // case immediately (in the same machine cycle).  We're not doing that (yet).
                        pc++;
                    }
                    break;
                case OpCode.LT:
                    {
                        Value b = instruction.B < regLimit ? registers[instruction.B] : constants[instruction.B - regLimit];
                        Value c = instruction.C < regLimit ? registers[instruction.C] : constants[instruction.C - regLimit];
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

                default:
                    throw new Exception("Unknown instruction: " + instruction.opcode);
            }
        }
    }
}

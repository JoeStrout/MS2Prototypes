using System;
using VM;
using Types;

namespace MS2Proto1
{
    class Program
    {

        static void Load42(VM.VM vm) {
            // Program to add 40 + 2 (using two variables, just for the sake of it)
            vm.constants.Add(new Value(40));
            vm.constants.Add(new Value(2));
            vm.registers.Add(Value.Null);
            vm.registers.Add(Value.Null);
            // a = 40
            vm.AddInstruction(new Instruction(OpCode.LOADK, 0, 0));  // R0 = K(0): 40
            // b = a + 2
            vm.AddInstruction(new Instruction(OpCode.ADD, 1, 0, VM.VM.regLimit+1));    // R1 = R0 + K1
            // move result to register 0
            vm.AddInstruction(new Instruction(OpCode.MOVE, 0, 1));
        }

        static void LoadSum(VM.VM vm) {
            // Program to sum the numbers 1 through N
            vm.constants.Add(new Value(5));  // starting value N
            vm.constants.Add(Value.Zero);
            vm.constants.Add(new Value(-1));

            vm.registers.Add(Value.Null);  // 0: sum
            vm.registers.Add(Value.Null);  // 1: i

            // sum = 0
            vm.AddInstruction(new Instruction(OpCode.LOADK, 0, 1));      // R0 = K(1)
            // i = N
            vm.AddInstruction(new Instruction(OpCode.LOADK, 1, 0));      // R1 = K(0)
            // (loop)
            // if i == 0 then goto end
            vm.AddInstruction(new Instruction(OpCode.EQ, 1, 1, VM.VM.regLimit+1));  // if R1 == K0 then...
            vm.AddInstruction(new Instruction(OpCode.JMP, 0, 4));  // ...jump +4
            // sum += i
            vm.AddInstruction(new Instruction(OpCode.ADD, 0, 0, 1));   // R0 = R0 + R1
            // i += -1
            vm.AddInstruction(new Instruction(OpCode.ADD, 1, 1, VM.VM.regLimit+2));    // R1 = R1 + K2
            // goto loop
            vm.AddInstruction(new Instruction(OpCode.JMP, 0, -4));     // jump -4
            // (end, result is already in register 0)
        }

        static void LoadFib(VM.VM vm) {
            // Program to compute the Nth Fibonacci number iteratively.
            // Reference: https://rosettacode.org/wiki/Fibonacci_sequence#MiniScript
            vm.constants.Add(new Value(6));     // K0. n (Fibonacci number to compute)
            vm.constants.Add(Value.Zero);       // K1
            vm.constants.Add(Value.One);        // K2
            vm.constants.Add(new Value(2));     // K3

            vm.registers.Add(Value.Null);       // R0. answer
            vm.registers.Add(Value.Null);       // R1. n1
            vm.registers.Add(Value.Null);       // R2. n2
            vm.registers.Add(Value.Null);       // R3. i

            // answer = n
            vm.AddInstruction(new Instruction(OpCode.LOADK, 0, 0));     // R0 = K(0)
            // if answer < 2 then goto done
            vm.AddInstruction(new Instruction(OpCode.LT, 1, 0, VM.VM.regLimit+3));  // if R(0) < K(3)...
            vm.AddInstruction(new Instruction(OpCode.JMP, 0, 10));  // ...then jump +10
            // n1 = 0
            vm.AddInstruction(new Instruction(OpCode.LOADK, 1, 1));  // R1 = K(1)
            // n2 = 1
            vm.AddInstruction(new Instruction(OpCode.LOADK, 2, 2));  // R2 = K(2)
            // i = n - 1
            vm.AddInstruction(new Instruction(OpCode.SUB, 3, VM.VM.regLimit+0, VM.VM.regLimit+2));   // R3 = K(0) - K(2)
            // if i < 1 then goto done
            vm.AddInstruction(new Instruction(OpCode.LT, 1, 3, VM.VM.regLimit+2));  // if R(1) < K(2)...
            vm.AddInstruction(new Instruction(OpCode.JMP, 0, 6));  // ...then jump +6
            // answer = n1 + n2
            vm.AddInstruction(new Instruction(OpCode.ADD, 0, 1, 2));        // R(0) = R(1) + R(2)
            // n1 = n2
            vm.AddInstruction(new Instruction(OpCode.MOVE, 1, 2));          // R(1) = R(2)
            // n2 = answer
            vm.AddInstruction(new Instruction(OpCode.MOVE, 2, 0));          // R(2) = R(0)
            // i -= 1
            vm.AddInstruction(new Instruction(OpCode.SUB, 3, 3, VM.VM.regLimit+2));    // R(3) = R(3) - K(2)
            // end for (goto top of loop)
            vm.AddInstruction(new Instruction(OpCode.JMP, 0, -6));          // pc -= 6
        }

        static void RunTest1() {
            var vm = new VM.VM();
            
            //Load42(vm);
            //LoadSum(vm);
            LoadFib(vm);

            while (true) {
                vm.PrintState();
                if (vm.IsDone()) break;
                Console.WriteLine("(Press any key to step.)");
                Console.ReadKey();
                vm.Step();
            }
            Console.WriteLine($"Program complete.  Result: {vm.registers[0]}");
        }

        static void Main(string[] args)
        {
            Console.Clear();
            RunTest1();
            // Console.WriteLine("Press any key to exit...");
            // Console.ReadKey();
        }
    }
} 
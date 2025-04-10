using System;
using VM;
using Types;

namespace MS2Proto1
{
    class Program
    {

        static void Load42(VM.VM vm) {
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

        static void RunTest1() {
            var vm = new VM.VM();
            
            //Load42(vm);
            LoadSum(vm);

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
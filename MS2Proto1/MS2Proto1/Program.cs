using System;
using VM;
using Types;

namespace MS2Proto1
{
    class Program
    {

        static void RunTest1() {
            var vm = new VM.VM();
            vm.constants.Add(new Value(42));
            vm.registers.Add(new Value(0));
            vm.registers.Add(new Value(0));
            vm.AddInstruction(new Instruction(OpCode.LOADK, 0, 0));    // R0 = K(0) ; 42
            vm.AddInstruction(new Instruction(OpCode.MOVE, 1, 0));    // R1 = R0
            while (true) {
                vm.PrintState();
                if (vm.IsDone()) break;
                Console.WriteLine("(Press any key to step.)");
                Console.ReadKey();
                vm.Step();
            }
            Console.WriteLine("Program complete.");
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
using System;
using System.Diagnostics;

namespace ScriptingVM.Benchmarks
{
    class FibRecursive
    {
        static Value Rfib(Value n_val)
        {
            if (!n_val.IsInt)
                return Value.Null();
            
            int n = n_val.AsInt();
            
            if (n <= 0)
                return Value.FromInt(0);
            if (n <= 2)
                return Value.FromInt(1);
            
            var n_minus_1 = Value.FromInt(n - 1);
            var n_minus_2 = Value.FromInt(n - 2);
            
            var fib1 = Rfib(n_minus_1);
            var fib2 = Rfib(n_minus_2);
            
            if (!fib1.IsInt || !fib2.IsInt)
            {
                Console.WriteLine($"ERROR: Non-integer result in rfib({n}): fib1={fib1.Bits:X} is_int={fib1.IsInt}, fib2={fib2.Bits:X} is_int={fib2.IsInt}");
                return Value.Null();
            }
            
            int val1 = fib1.AsInt();
            int val2 = fib2.AsInt();
            int result_val = val1 + val2;
            return Value.FromInt(result_val);
        }
        
        static void RunBenchmark(int n)
        {
            var n_val = Value.FromInt(n);
            
            Console.WriteLine($"Testing with n={n}, n_val=0x{n_val.Bits:X}, as_int={n_val.AsInt()}");
            
            var stopwatch = Stopwatch.StartNew();
            var result = Rfib(n_val);
            stopwatch.Stop();
            
            Console.WriteLine($"rfib({n}) = {result.AsInt()}, time: {stopwatch.ElapsedMilliseconds / 1000.0:F3} seconds");
        }
        
        static void Main(string[] args)
        {
            Console.WriteLine("C# NaN Boxing Fibonacci Benchmark");
            Console.WriteLine("==================================");
            
            Console.WriteLine("Testing small cases:");
            for (int i = 0; i <= 5; i++)
            {
                var n_val = Value.FromInt(i);
                var result = Rfib(n_val);
                Console.WriteLine($"rfib({i}) = {result.AsInt()}");
            }
            
            Console.WriteLine("\nBenchmark results:");
            RunBenchmark(30);
            
            // Show handle pool usage
            Console.WriteLine($"\nHandlePool usage: {HandlePool.GetCount()} objects");
        }
    }
}
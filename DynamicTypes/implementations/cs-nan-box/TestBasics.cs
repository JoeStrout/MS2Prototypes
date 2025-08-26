using System;
using System.Diagnostics;

namespace ScriptingVM.Tests
{
    class TestBasics
    {
        static void TestValueCreation()
        {
            Console.WriteLine("Testing Value creation...");
            
            // Test null
            var nullVal = Value.Null();
            Debug.Assert(nullVal.IsNull);
            Debug.Assert(!nullVal.IsInt);
            Debug.Assert(!nullVal.IsDouble);
            Debug.Assert(!nullVal.IsString);
            
            // Test integer
            var intVal = Value.FromInt(42);
            Debug.Assert(intVal.IsInt);
            Debug.Assert(!intVal.IsNull);
            Debug.Assert(!intVal.IsDouble);
            Debug.Assert(intVal.AsInt() == 42);
            
            // Test double
            var doubleVal = Value.FromDouble(3.14);
            Debug.Assert(doubleVal.IsDouble);
            Debug.Assert(!doubleVal.IsInt);
            Debug.Assert(!doubleVal.IsNull);
            Debug.Assert(Math.Abs(doubleVal.AsDouble() - 3.14) < 0.0001);
            
            // Test tiny string
            var tinyStr = Value.FromString("hi");
            Debug.Assert(tinyStr.IsString);
            Debug.Assert(tinyStr.IsTiny);
            Debug.Assert(!tinyStr.IsHeapString);
            Debug.Assert(tinyStr.ToString() == "hi");
            
            // Test heap string
            var heapStr = Value.FromString("this is a longer string");
            Debug.Assert(heapStr.IsString);
            Debug.Assert(!heapStr.IsTiny);
            Debug.Assert(heapStr.IsHeapString);
            Debug.Assert(heapStr.ToString() == "this is a longer string");
            
            Console.WriteLine("✓ Value creation tests passed");
        }
        
        static void TestArithmetic()
        {
            Console.WriteLine("Testing arithmetic operations...");
            
            var a = Value.FromInt(10);
            var b = Value.FromInt(5);
            var c = Value.FromDouble(2.5);
            
            // Integer addition
            var sum1 = Value.Add(a, b);
            Debug.Assert(sum1.IsInt);
            Debug.Assert(sum1.AsInt() == 15);
            
            // Integer subtraction
            var diff1 = Value.Sub(a, b);
            Debug.Assert(diff1.IsInt);
            Debug.Assert(diff1.AsInt() == 5);
            
            // Mixed int/double addition
            var sum2 = Value.Add(a, c);
            Debug.Assert(sum2.IsDouble);
            Debug.Assert(Math.Abs(sum2.AsDouble() - 12.5) < 0.0001);
            
            // Mixed int/double subtraction  
            var diff2 = Value.Sub(a, c);
            Debug.Assert(diff2.IsDouble);
            Debug.Assert(Math.Abs(diff2.AsDouble() - 7.5) < 0.0001);
            
            Console.WriteLine("✓ Arithmetic tests passed");
        }
        
        static void TestComparison()
        {
            Console.WriteLine("Testing comparison operations...");
            
            var int1 = Value.FromInt(10);
            var int2 = Value.FromInt(10);
            var int3 = Value.FromInt(5);
            var double1 = Value.FromDouble(10.0);
            var str1 = Value.FromString("hello");
            var str2 = Value.FromString("hello");
            var str3 = Value.FromString("world");
            
            // Integer equality
            Debug.Assert(Value.Equal(int1, int2));
            Debug.Assert(!Value.Equal(int1, int3));
            
            // Mixed int/double equality
            Debug.Assert(Value.Equal(int1, double1));
            
            // String equality
            Debug.Assert(Value.Equal(str1, str2));
            Debug.Assert(!Value.Equal(str1, str3));
            
            // Less than
            Debug.Assert(Value.LessThan(int3, int1));
            Debug.Assert(!Value.LessThan(int1, int3));
            Debug.Assert(!Value.LessThan(int1, int2));
            
            Console.WriteLine("✓ Comparison tests passed");
        }
        
        static void TestTinyStrings()
        {
            Console.WriteLine("Testing tiny string operations...");
            
            // Test various lengths
            var empty = Value.FromString("");
            var one = Value.FromString("a");
            var five = Value.FromString("hello");
            
            Debug.Assert(empty.IsTiny);
            Debug.Assert(empty.TinyLen() == 0);
            Debug.Assert(empty.ToString() == "");
            
            Debug.Assert(one.IsTiny);
            Debug.Assert(one.TinyLen() == 1);
            Debug.Assert(one.ToString() == "a");
            
            Debug.Assert(five.IsTiny);
            Debug.Assert(five.TinyLen() == 5);
            Debug.Assert(five.ToString() == "hello");
            
            // Test non-ASCII (should go to heap)
            var unicode = Value.FromString("café");
            Debug.Assert(!unicode.IsTiny);
            Debug.Assert(unicode.IsHeapString);
            Debug.Assert(unicode.ToString() == "café");
            
            Console.WriteLine("✓ Tiny string tests passed");
        }
        
        static void TestToString()
        {
            Console.WriteLine("Testing ToString() formatting...");
            
            Debug.Assert(Value.Null().ToString() == "null");
            Debug.Assert(Value.FromInt(42).ToString() == "42");
            Debug.Assert(Value.FromDouble(3.14).ToString().StartsWith("3.14"));
            Debug.Assert(Value.FromString("hello").ToString() == "hello");
            
            Console.WriteLine("✓ ToString tests passed");
        }
        
        static void Main(string[] args)
        {
            Console.WriteLine("C# NaN Boxing Value Tests");
            Console.WriteLine("========================");
            Console.WriteLine();
            
            TestValueCreation();
            TestArithmetic();
            TestComparison();
            TestTinyStrings();
            TestToString();
            
            Console.WriteLine();
            Console.WriteLine("🎉 All tests passed!");
        }
    }
}
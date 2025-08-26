using System;
using ScriptingVM;

class TestDebug
{
    static void Main()
    {
        Console.WriteLine("Debug C# NaN Boxing");
        Console.WriteLine("==================");
        
        // Test null value
        var nullVal = Value.Null();
        Console.WriteLine($"Null: bits=0x{nullVal.Bits:X16} IsNull={nullVal.IsNull}");
        
        // Test integer
        var intVal = Value.FromInt(42);
        Console.WriteLine($"Int(42): bits=0x{intVal.Bits:X16} IsInt={intVal.IsInt}");
        
        // Test tiny string
        var tinyStr = Value.FromString("hi");
        Console.WriteLine($"String('hi'): bits=0x{tinyStr.Bits:X16} IsTiny={tinyStr.IsTiny} IsHeapString={tinyStr.IsHeapString} IsString={tinyStr.IsString}");
        Console.WriteLine($"  Expected tiny mask: 0x{0xFFFF_0000_0000_0000UL:X16}");
        Console.WriteLine($"  Actual top 16 bits: 0x{(tinyStr.Bits & 0xFFFF_0000_0000_0000UL):X16}");
        
        // Test double
        var doubleVal = Value.FromDouble(3.14);
        Console.WriteLine($"Double(3.14): bits=0x{doubleVal.Bits:X16} IsDouble={doubleVal.IsDouble}");
        
        Console.WriteLine("\nMasks:");
        Console.WriteLine($"NANISH_MASK:  0xFFFF000000000000");
        Console.WriteLine($"INTEGER_MASK: 0x7FFC000000000000"); 
        Console.WriteLine($"STRING_MASK:  0xFFFE000000000000");
        Console.WriteLine($"LIST_MASK:    0xFFFD000000000000");
        Console.WriteLine($"MAP_MASK:     0xFFFB000000000000");
        Console.WriteLine($"TINY_MASK:    0xFFFF000000000000");
        Console.WriteLine($"NULL_VALUE:   0x7FFE000000000000");
    }
}
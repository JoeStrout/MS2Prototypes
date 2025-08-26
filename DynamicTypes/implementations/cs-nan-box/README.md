# C# NaN Boxing Dynamic Type System

A C# implementation of a NaN boxing dynamic type system, mirroring the functionality of the C implementation in `c-nan-boxing-3`. This system uses IEEE 754 NaN bit patterns to pack multiple data types into a single 64-bit value.

## Features

- **NaN Boxing**: Stores null, integers, doubles, and object references in a single 64-bit `Value` struct
- **Tiny String Optimization**: Strings ≤5 ASCII characters are stored inline (no heap allocation)
- **HandlePool**: Object table for heap-allocated strings, lists, and other objects
- **Dynamic Lists**: Generic `List<Value>` supporting mixed-type collections
- **String Operations**: Complete set of string manipulation functions (split, replace, indexOf, concat)
- **Type Coercion**: Automatic int/double promotion in arithmetic operations

## Building and Running

### Prerequisites
- .NET 6.0 or later
- OR Mono (6.12+) with `mcs` compiler

### Using .NET (Recommended)
```bash
# Create and build test project
dotnet new console -n NanBoxTests -f net6.0
cp Value.cs NanBoxTests/
cp [TestFile].cs NanBoxTests/Program.cs
dotnet build NanBoxTests
dotnet run --project NanBoxTests
```

### Using Mono
```bash
# Compile and run directly
mcs -out:test.exe Value.cs [TestFile].cs
mono test.exe
```

## Test Files

- **`TestBasics.cs`**: Core functionality (value creation, arithmetic, comparison)
- **`TestStringOperations.cs`**: String and list operations
- **`FibRecursive.cs`**: Recursive Fibonacci benchmark
- **`LevenshteinImproved.cs`**: Edit distance algorithm using string/list operations
- **`NumberWordsImproved.cs`**: Number-to-text conversion using dynamic string operations

## Benchmark Results

All benchmarks use the complete NaN boxing Value system with dynamic string/list operations:

| Benchmark | Test Case | Time | Mini Micro Time | Notes |
|-----------|-----------|------|-------|
| **Fibonacci** | `fib(30)` | **0.045s** | 6.831s | Pure integer computation |
| **Levenshtein** | Full test suite | **0.284s** | 26.907s | String splitting, list operations, character comparison |
| **NumberWords** | 10,000 conversions | **0.141s** | 1.361s | String concatenation, splitting, replace operations |

All benchmarks were run on a 2.3 GHz 8-Core Intel Core i9 running MacOS 13.6.4, using dotnet 6.0.106.  Equivalent Mini Micro benchmarks were run on the same machine, within Mini Micro 1.2.5.  Note that the benchmarks above exercise only the type system; they do not include a MiniScript VM, and so serve only as upper bounds to the performance possible.

### Correctness Verification

- **Fibonacci**: Produces correct sequence (832040 for n=30)
- **Levenshtein**: Passes classic "kitten" → "sitting" = 3, handles complex text like Gettysburg Address variants
- **NumberWords**: Perfect round-trip accuracy for all test cases including negatives and large numbers

## Architecture

### Value Structure
```csharp
[StructLayout(LayoutKind.Explicit, Size = 8)]
public readonly struct Value
{
    [FieldOffset(0)] private readonly ulong _u;  // Bit patterns
    [FieldOffset(0)] private readonly double _d; // IEEE 754 double
}
```

### Type Encoding
- **Doubles**: Native IEEE 754 representation
- **Integers**: `0x7FFC_0000_0000_0000 | (uint)value`
- **Tiny Strings**: `0xFFFF_0000_0000_0000 | length | char_data`
- **Heap Objects**: `TAG_MASK | handle_index`

### Memory Management
- **Tiny strings**: Zero heap allocations for short ASCII strings
- **HandlePool**: Simple object table for heap references
- **No GC integration**: Relies on .NET's garbage collector for heap objects

## Compatibility

- **Unity**: Compatible with Unity 2021.3+ (.NET Standard 2.1)
- **Older Mono**: Uses `BitConverter` instead of `Unsafe` for broader compatibility
- **IL2CPP**: Should work with IL2CPP compilation

## Performance Notes

The C# implementation prioritizes correctness and compatibility over raw performance. For production use, consider:
- Implementing object pooling for frequently created Values
- Using `Unsafe` operations on newer runtimes
- Adding SIMD optimizations for bulk operations
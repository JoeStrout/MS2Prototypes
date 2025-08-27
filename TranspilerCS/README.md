# TranspilerCS

A C# to C++ transpiler that converts a restricted subset of C# programs to lean, STL-free C++, based on the C# Compiler Platform (Roslyn).

## What It Does

TranspilerCS takes C# source code and generates equivalent C++ code with the following transformations:

- **Properties** → getter/setter methods with backing fields
- **`foreach` loops** → traditional `for` loops with indexing  
- **Field declarations** → proper C++ syntax with correct access modifiers
- **Property references** → backing field access (`Consts` → `__prop_Consts`)
- **Mini.List<T>** → C++ container classes (assumes runtime is available)

The output is designed to be lean and avoid pulling in the STL, assuming certain types and container classes are already available in your target environment.

## Why Would We Do This?

For MiniScript 2.0, we would like to avoid maintaining two versions (C# and C++) of the code by hand.  But we still need to serve both C# and C++ embedders.  So, we are considering writing the bulk of the compiler and interpreter in a carefully restricted subset of C#, and then using a transpiler like this to auto-generate equivalent C++ code.

Core types, and anywhere we are doing deep C or C# magic for the sake of performance, will still be hand-written in each language, but the hope is that these will be a small and more easily maintained subset of the whole system.

## Requirements

- .NET 6.0 or later
- C# source code using only the supported subset

## Building

```bash
dotnet build
```

## Running

```bash
dotnet run [input-file]
```

- **Default:** Processes `tests/Sample.cs` 
- **Custom:** Specify any `.cs` file path
- **Output:** Generates `generated/[filename].g.cpp`

## Output

The transpiler generates:
- `generated/[filename].g.h` - C++ header file with class declarations
- `generated/[filename].g.cpp` - C++ implementation file with method definitions

You must provide:
- `mini_runtime.hpp` - Runtime definitions for Mini types (`Mini::List<T>`, `Mini::Value`, etc.)

## Supported C# Features

- Classes with fields and methods
- Auto-properties (with get/set)
- `foreach` loops over `Mini.List<T>`
- Basic field access modifiers (`public`, `private`)
- Method calls and property access
- Simple expressions and statements

## Not yet supported

- Class references, in particular converting `.` to `->` or `::` as appropriate
- Adding `()` where needed on computed properties
- Probably a bunch of other stuff


## Architecture

- **PortableSubsetChecker** - Validates that input uses only supported C# constructs
- **CppRewriter** - Transforms C# syntax tree to C++-compatible forms
- **CppEmitter** - Generates final C++ source code text
- **Syntactic Approach** - Uses syntax-based transformations for reliable output
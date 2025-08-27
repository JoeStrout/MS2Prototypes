# TranspilerMS

A C# to C++ transpiler that converts a restricted subset of C# programs to lean, STL-free C++, written in MiniScript.

## What It Does

The transpile.ms program in this directory takes C# source code and generates equivalent C++ code with the following transformations:

- **using** → `#include`
- **Field declarations** → proper C++ syntax with correct access modifiers
- **Properties** → getter/setter methods with backing fields
- **`foreach` loops** → traditional `for` loops with indexing  
- **Mini.List<T>** → C++ container classes (assumes runtime is available)

The output is designed to be lean and avoid pulling in the STL, assuming certain types and container classes are already available in your target environment.

## Why Would We Do This?

For MiniScript 2.0, we would like to avoid maintaining two versions (C# and C++) of the code by hand.  But we still need to serve both C# and C++ embedders.  So, we are considering writing the bulk of the compiler and interpreter in a carefully restricted subset of C#, and then using a transpiler like this to auto-generate equivalent C++ code.

Core types, and anywhere we are doing deep C or C# magic for the sake of performance, will still be hand-written in each language, but the hope is that these will be a small and more easily maintained subset of the whole system.

## Requirements

- MiniScript
- C# source code using only the supported subset

## Running

```bash
miniscript transpiler.ms [input-file]
```

- **Default:** Processes `tests/Sample.cs` 
- **Custom:** Specify any `.cs` file path (not yet supported)
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

Like TranspilerCS, this transpiler uses a syntactic approach, mostly using simple pattern matching and some state/context data.  The bulk of the logic is in Converter.processLine, which pulls the line apart into indentation, meat, and trailing whitespace/comment, then matches and transforms the meat of the line, pasting the indentation and trailing stuff back upon output.

This is essentially the same thing TranspilerCS does, but the MiniScript code seems a lot more straightforward, making it easier to write and maintain.

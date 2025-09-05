# MS2Prototypes

### Prototyping work for MiniScript 2.0

This repo contains a variety of small prototypes developing techniques we are likely to need for version 2.0 of [MiniScript](https://miniscript.org).

MiniScript 2.0 will be a complete rewrite, focused on performance.  The current (1.x) implementation is clean and simple, but does poorly on common [benchmarks](github.com/JoeStrout/miniscript-benchmarks).  We intend to fix that in 2.0 by focusing on performance first, while still maintaining the same program behavior.

The subdirectories here, in order of creation, are:

- MS2Proto1: an early prototype of a clean VM in C#.
- DynamicTypes: experiments with NaN boxing, garbage collection (GC), and string interning in both C and C#, with the goal of developing a dynamic type system (a `Value` that can hold any MiniScript type) that is fast and memory-efficient.  Includes some benchmark programs in MiniScript, and in C/C# using the new type systems.
- TinyVM: prototype of a VM written in C with a sophisticated opcode dispatching system, using a "computed goto" technique when compiled with compatible C compilers, and falling back to an ordinary `switch` statement elsewhere.
- MS2Proto2: a complete VM and assembler in C, based on the best NaN-boxing prototype and the TinyVM prototype.  It includes several demo/benchmark programs, in assembly language.
- MS2Proto3: our newest prototype, focused on simultaneous development of C# and C++ code via transpiler.ms.
- TranspilerCS: an attempt to "transpile" (convert) code from C# to C++, using the C# compiler infrastructure (aka Roslyn).
- TranspilerMS: the same thing as TranspilerCS, but written in MiniScript.  This version is considerably easier to write and maintain, and seems to be working better too.  The goal here is to enable us to write MiniScript 2.0 mostly in C#, with auto-generated code for the C++ version.


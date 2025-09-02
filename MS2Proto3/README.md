# MiniScript 2.0 Prototype 3

This subproject is our first attempt to write code that might actually make it into MiniScript 2.0.  It will use the C# -> transpiler -> C++ pipeline to generate working projects in both C# and C++ from the get go; it will use memory management; and it will use our NaN-boxed `Value` type system.

## Memory Management

The C++ code requires careful thinking about memory management.  This project combines two different strategies:

1. For **runtime values**, i.e. the `Value` type system, strings/lists/maps are managed by the garbage collection (GC) system.  All code that touches these types needs to use `GC_PROTECT` on its locals in order to maintain the shadow stack that GC requires to work (or else, it must temporarily disable GC while doing any GC allocations).

2. For all other **host app values**, such as strings/lists/maps used by the compiler or assembler to process raw source code, we will use a `MemPool` system.  Allocations go into a particular pool, where they are held forever until that pool is deallocated.  A "reference" in this system is a combination of *pool_id*, *block_id*, and should safely map to `NULL` after the pool is drained, at least until the pool ID is reused (which should be very rare).

Developing and validating this dual-management strategy is an important goal of this prototype.

## Development Milestones

1. ✅ **Main Program Stub**: runs, prints some program info, reads a line from stdin, and prints something about it to stdout.  This is just to get our directory structure and toolchain up and running.

2. **Assembler/Disassembler**: reads a .msa (MiniScript Assembly) file from disk or stdin, and writes a .mso (object) file out; or vice versa.  Note that this does not yet require any `Value`s.  It might (or might not!) require adding a Dictionary type to our core types.

3. **Minimal VM**: reads a .msa or .mso file, assembling the .msa on the fly, and then executes it.  The VM at this point will support only a handful of opcodes.

4. **Extended VM**: same as above, but with what we believe to be a complete set of opcodes, including calls and captures.

Note that the compiler is beyond scope for this prototype -- but if we make it through milestone 4, we'll probably move all this over to its own fresh GitHub project, and start working on that.

## Coding Standards & Restrictions

As much as possible, code should be written in .cs (C#) files.  There will be a set of core classes/modules in C++ supporting the common types, such as String and List, and the support code those require (e.g. memory management).  But all the logic of the assembler, disassembler, and VM should be written in C# only, and then run through transpiler.ms to output equivalent C++ code.

To make this work, we place rather strict requirements on the .cs code.  See [CS Coding Standards](CS_CODING_STANDARDS.md).

## How to Build and Test

All the management is done by the `tools/build.sh` shell script.  (You will need an environment that can run bash scripts to use this, of course.)

1. `cd MS2Proto3` if you're not already in the right folder
2. `tools/build.sh setup` to set up your folders (you only need to do this once)
3. `tools/build.sh cs` to compile the C# code
4. `tools/build.sh transpile` to convert C# to C++ code
5. `tools/build.sh cpp` to compile the C++ code
6. `tools/build.sh test` to run both projects; or you can manually run `build/cs/MS2Proto3` or `build/cpp/MS2Proto3`

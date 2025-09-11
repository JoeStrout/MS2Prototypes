# MiniScript 2.0 Prototype 3

This subproject is our first attempt to write code that might actually make it into MiniScript 2.0.  It will use the C# -> transpiler -> C++ pipeline to generate working projects in both C# and C++ from the get go; it will use memory management; and it will use our NaN-boxed `Value` type system.

## Memory Management

The C++ code requires careful thinking about memory management.  This project combines two different strategies:

1. For **runtime values**, i.e. the `Value` type system, strings/lists/maps are managed by the garbage collection (GC) system.  All code that touches these types needs to use `GC_PROTECT` on its locals in order to maintain the shadow stack that GC requires to work (or else, it must temporarily disable GC while doing any GC allocations).

2. For all other **host app values**, such as strings/lists/maps used by the compiler or assembler to process raw source code, we will use a `MemPool` system.  Allocations go into a particular pool, where they are held forever until that pool is deallocated.  A "reference" in this system is a combination of *pool_id*, *block_id*, and should safely map to `NULL` after the pool is drained, at least until the pool ID is reused (which should be very rare).

Developing and validating this dual-management strategy is an important goal of this prototype.

## Development Milestones

1. ✅ **Main Program Stub**: runs, prints some program info, reads a line from stdin, and prints something about it to stdout.  This is just to get our directory structure and toolchain up and running.

2. ✅ **Assembler/Disassembler**: reads a .msa (MiniScript Assembly) file from disk or stdin, assembles it to bytecode, disassembles the bytecode, and prints to stdout.  Note that this will require some `Value` support, for the constants table.  It might (or might not!) require adding a Dictionary type to our core types.

3. ✅ **Minimal VM**: reads a .msa file, assembles it, and then executes it.  The VM at this point will support only a handful of opcodes.

4. **Extended VM**: same as above, but with what we believe to be a complete set of opcodes, including calls and captures.  It will also include the computed-goto dispatch method in the C++ version, as well as runtime maps and lists.

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
5. `tools/build.sh cpp` to compile the C++ code (add `auto`, `on`, or `off` to control computed-goto feature)
6. `tools/build.sh test` to run both projects; or you can manually run `build/cs/MS2Proto3` or `build/cpp/MS2Proto3`

## To-Do List

Here are some of the things we'll need to solve, implement, or clean up as we wrap up Milestone 4 and move to production code.

- Implement maps, lists, and the opcodes needed for them.
- Find a way to invoke intrinsics -- possibly a temporary method for now -- to give us a `print` functionality, plus a handful of numerics that might be useful for benchmarking.
- Implement a dictionary data structure for use in the host code (analogous to C# Dictionary).
- Get all performance-critical code in the VM execution loop inlined.
- See how much we can unify (transpile) the Value code from C# to C, since their memory layout should be the same.  (Without sacrificing the C rather than C++ interface!)
- Ensure that all our type operations behave the same as in MiniScript 1.0: proper fuzzy-logic operators, multiplication of strings/lists by fractional values, etc.
- Audit and/or measure use of the two heap managers (MemPool and GC), and ensure that these are being used correctly & consistently.
- Fix our string interning.  For example, if the constant "abcdef" appears in a function twice, it should appear in the constants table only once, because it gets interned and wrapped up as the exact same Value.  But that's not currently happening.
- Compare performance of all three VMs (C#, and C++ with/without computed goto) to equivalent MiniScript programs, and ensure we're still in the target zone (100X - 500X faster than MiniScript 1.0).
- Figure out if we really want/need Value to be able to contain Int32's, or if we can do everything with doubles as in MS1.
- Improve and standardize error handling in the assembler (particularly with regard to out-of-bounds numeric literals).
- Figure out how to map registers to local variables, and how to make `locals` work.
- Add lookup-by-name and assign-by-name opcodes.
- Figure out closures (i.e. `outer`).
- Figure out tail call optimization.
- Maybe create a debugger that lets us single-step (or Step Over, etc.) the VM, displaying machine state on each pause.  Or maybe that's overkill.




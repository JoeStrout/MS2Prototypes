# MS2Proto1

## Goals:

- Simple VM supporting numbers, strings, and basic operations thereon
- Basic flow control (_if_, _jump_)
- Manual assembly of instructions, in C# code
- Single-step and display of machine state
- Support of simple call and return
- Demonstrate _sum_, _fib_ (iterative fibonacci), and _rfib_ (retursive fibonacci) functions

## Progress Log

### 10 Apr 2025

We have a simple VM with a handful of instructions; enough to load constants into registers, do addition with them,
test for equality, and jump (conditional or unconditional).  We can poke a program into the machine, and step through
the execution one instruction at a time, printing the machine state on each step.

We've implemented the _sum_ demo and _fib_ demos.

So, the next big step is function calls, and the _rfib_ test program.

We should also start thinking about a better assembly language format.
What we have right now is a very direct translation of the instruction format, but we could be slightly less direct and
make it a lot easier to read and write.


### 11 Apr 2025

Added support for function calls, and implemented _rfib_.  Seems to work.

In this prototype, function code lives in the same VM instruction list as the global code.  Functions are not
first-class objects.  In the next prototype, let's tackle that: we need a function datatype that either contains
the code, or knows where in the machine instruction set the code lives.

We should also start thinking about lists and maps — especially maps, and how we're going to index into them,
and how this relates to locals which can be accessed via the `locals` map while still being stored in registers.

Or, maybe the next prototype just improves the assembly format and gives us a mini-assembler we can work in
interactively.

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

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

Well, it's a start.  We have just three instructions: LOADK, LOADNIL, and MOVE.  With these we can run a program
equivalent to `a = 42; b = a`, stepping through one instruction at a time and dumping the machine state.

The next step should probably be to add JUMP and IF, which should be enough to write the _sum_ and _fib_ functions (reading the result directly out of the final register).

I might like to also improve the machine state display a bit, printing registers and constants next to each other in two columns, just to make it neater and easier to read.  We should also start thinking about a better assembly language format.

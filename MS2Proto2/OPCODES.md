# Opcodes in this VM

| Mnemonic | A | B | C | Notes |
| --- | --- | --- | --- | --- |
| MOVE | R_dest | R_src | - | R[A] := R[B] |
| LOADN | R_dest | int16 | ← | R[A] := BC (16-bit signed value) |
| LOADK | R_dest | const_idx | ← | R[A] := constants[BC] (load from constants table) |
| ADD | R_dest | R_op1 | R_op2 | R[A] := R[B] + R[C] |
| SUB | R_dest | R_op1 | R_op2 | R[A] := R[B] - R[C] |
| MULT | R_dest | R_op1 | R_op2 | R[A] := R[B] * R[C] |
| DIV | R_dest | R_op1 | R_op2 | R[A] := R[B] / R[C] |
| JUMP | - | offset | ←| PC += BC (16-bit signed value) |
| IFLT | R_a | R_b | offset | if R[A] < R[B] then PC += offset (8-bit signed) |
| CALLF | argWinStart | numArgs | funcIdx | call funcs[funcIdx] |
| RETURN | - | - | - | return with result in R[0]

### Pseudo-opcodes

| Mnemonic | A | B | C | Notes |
| --- | --- | --- | --- | --- |
| LOAD | R_dest | constant | - | Auto-selects LOADK or LOADN based on constant type/size |

The `LOAD` pseudo-opcode automatically chooses the most efficient loading instruction:
- If the constant is a 16-bit signed integer (-32768 to 32767), compiles as `LOADK`
- Otherwise (large integers, doubles, strings), compiles as `LOADN`

### Notes on CALLF

Calls work by passing all registers past some point in the current call frame to the called function, as arguments.  Upon return, the first of those will contain the result, and any registers beyond that are potentially clobbered.  So for example:

```
   CALLF 3, 2, 42
```
Means that we're calling function 42 (in our VM's list of functions), passing arguments in R3 and R4.  After the call, the result will be found in R3, and registers R4, R5, etc. should be considered to contain garbage.

Note that the current VM does not actually use the numArgs argument.  It might be handy to keep around for debugging, or we could lose it, and use BC for the function index, increasing the number of possible functions from 256 to 65536.

# Function Calls in the MiniScript 2 VM

## Considerations

- We can never know at compile time what parameters the callee expects.
- Caller may pass fewer arguments than callee expects; the remaining parameters get default values.
- Caller may pass more arguments than callee expects; this should raise a runtime error.
- Arguments are (at least for now) always passed by position; we do not support named parameters.
- Callee stack space beyond the argument registers must be cleared at least enough to ensure that it never encounters stale data.  This includes both register values, and register (local variable) names.
- We want call overhead to be as low as possible (this has been a key performance bottleneck in MiniScript 1.x).

## Overview of Approach

- Our FuncDef objects will include parameter names and default values.  (In MiniScript assembly, these can be specified with a ".param" directive.)
- A function invocation without arguments (a very common case) is a simple CALL opcode, specifying where on the stack the callee's register window should begin.  Execution of that CALL opcode will initialize all arguments to their default values and clear subsequent registers as needed.
- To invoke a function with arguments, we emit a block of opcodes that are all executed at once.  These begin with a ARGBLOCK opcode, which specifies how many argument instructions follow; then an ARG opcode for each one; and finally the CALL opcode.

## Example

Here's a function that takes three parameters, `a`, `b`, and `c`, with default values of `None`, `1`, and `0` respectively.  It returns the sum.

```
@add:
	.param a
	.param b=1
	.param c=0
	ADD r0, r0, r1
	ADD r0, r0, r2
	RETURN
```

To call `add(40, 2)` might look like this:

```
	LOADV r0, r0, "add"   # find funcref from identifier "add", store in r0
	ARGBLOCK 2
	ARG 40
	ARG 2
	CALL r1, r4, r0    # call func in r0, with window at r4, storing result in r1
```

When the VM hits the `ARGBLOCK 2` instruction, it does a bunch of things:

1. Look ahead 2 additional instructions (past the current PC, which would already be on ARG 40) to find the CALL.  
2. Look up the FuncDef, discovering that it has three parameters.  If argCount > paramCount, throw a runtime error.  (No error in this example since 2 <= 3.)
3. Advance the pc quickly through the ARG instructions, copying those values into registers r4 and r5, while naming them `a` and `b` from the FuncDef info.
4. Set up additional parameters, in this case, store 0 in r6 and name it `c`.
5. Clear any additional registers the callee needs (in this case, none are needed).
6. Shift the register window to what's currently r4, and push the new CallInfo onto the call stack.  (This call info includes the @add bytecode, and a note to store the result in r1.)

When, inside the @add function, the VM hits the RETURN statement:

1. Grab the result from r0.
2. Pop the call stack, shifting the register window back down.
3. Store the result in r1.

## No Arguments?

If there are no arguments, then we don't have an `ARGBLOCK` instruction; the VM would see a naked `CALL`.  In this case it can skip steps 1-3 above, but still do steps 4-6.  Return handling is unchanged.

## Named Parameters?

If we decide to add support for named parameters, then we'll just have an alternate form of the ARG opcode that includes a name, e.g. `ARG 7, "c"`.  We'll require such named arguments to come after any unnamed ones, and the VM will validate and store those values as part of the above call process.

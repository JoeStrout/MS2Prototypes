## Tiny VM Demo

This is a demo of a tiny VM built for speed, using some gnarly C macros (created with the help of ChatGPT 5) for dispatching opcodes.  These will use a computed `goto` statement on platforms which support it (GCC and Clang), but fall back to an ordinary `switch` elsewhere.

## Building

- `cc -O3 -DVM_USE_COMPUTED_GOTO=1 tiny_vm_fib.c -o tinyvm` (for computed `goto`)
- `cc -O3 -DVM_USE_COMPUTED_GOTO=0 tiny_vm_fib.c -o tinyvm ` (for `switch`)

Note that if the VM_USE_COMPUTED_GOTO switch is not specified in the compile command, it will attempt to auto-detect whether computed goto is supported and compile accordingly.

## Results

On my machine (2.3 GHz 8-Core Intel Core i9 running MacOS 13.6.4), this VM computes rfib(40) = 102334155 as follows:

| Condition | Result |
| ----- | ----- |
| with computed goto | 3.13s user 0.01s system 99% cpu 3.148 total |
| without computed goto | 4.84s user 0.01s system 99% cpu 4.866 total |

So the computed-goto approach is about 35% faster on this benchmark.  (But both results are screamingly fast -- see below.)

Here's how this VM compares to MiniScript, Python, and Lua.  (Note that this time includes startup/compilation time for the real languages.)

| Platform | Time |
| ----- | ----- |
| MiniScript | 1717s |
| Python | 35.95s |
| Lua | 9.77s |
| TinyVM | 3.13s |

## Caveats

The `Value` type in this VM is just an `int`.  It doesn't have to deal with dynamic types at all.  And its instruction set is very small, which helps with instruction cache.  So we probably can't achieve this level of performance in a real VM — but these results provide an upper bound on what we might be able to do.

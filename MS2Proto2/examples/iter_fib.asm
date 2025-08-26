# Simple iterative Fibonacci calculation
# Computes fib(n) where n is loaded as a constant
# Result stored in r0

LOADK r0, 7      # n = 7 (compute fib(7))
LOADK r1, 0      # a = 0 (first Fibonacci number)
LOADK r2, 1      # b = 1 (second Fibonacci number)

# Handle base cases
LOADK r3, 1      # Load 1 for comparison
IFLT r0, r3, zero # if n < 1, return 0
LOADK r3, 2      # Load 2 for comparison  
IFLT r0, r3, one  # if n < 2, return 1

# Iterative loop: for i = 2 to n
LOADK r4, 2      # i = 2 (loop counter)

loop:
    ADD r3, r1, r2   # r3 = a + b (next Fibonacci number)
    MOVE r1, r2      # a = b
    MOVE r2, r3      # b = r3
    LOADK r3, 1      # Load 1 for increment
    ADD r4, r4, r3   # i = i + 1
    IFLT r0, r4, end  # if n < i, exit (i.e., if i > n, exit)
    JMP loop         # Continue loop

zero:
    LOADK r0, 0      # Return 0
    RETURN

one: 
    LOADK r0, 1      # Return 1
    RETURN

end:
    MOVE r0, r2      # Result is in r2
    RETURN
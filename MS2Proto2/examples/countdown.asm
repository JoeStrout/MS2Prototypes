# Countdown loop from 5 to 0
# Demonstrates labels and conditional jumps

LOADK r0, 5      # Start with 5

loop:            # Loop label
    LOADK r1, 0      # Load 0 for comparison
    IFLT r0, r1, end # if r0 < 0, jump to end
    LOADK r1, 1      # Load 1 for decrement
    SUB r0, r0, r1   # r0 = r0 - 1
    JMP loop         # Jump back to loop

end:             # End label
    RETURN           # Return final value
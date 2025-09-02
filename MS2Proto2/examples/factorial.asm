@factorial:
    # Constants
    LOAD r1, 1 
    LOAD r2, 2
    
    # Check for base cases of 1 or 0
    IFLT r0, r2, lessTwo

    # Not base case. Calculate normal factorial recursively
    SUB r3, r0, r1
    CALLF r3, 1, 0
    MULT r0, r0, r3
    RETURN
lessTwo:
    # Base cases. r0 is less than 2, so we return 1
    LOAD r0, 1 # alternatively, MOVE r0, r1
    RETURN

@main:
    LOAD r0, 5 # We're going with factorial of 5 for now
    CALLF r0, 1, 0
    RETURN
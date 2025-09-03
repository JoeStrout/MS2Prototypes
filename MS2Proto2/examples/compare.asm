# A program that tests the comparison operators.
#  0 -> Working
# -1 -> Not working

@main:
    LOAD r0, 0
    LOAD r1, 1

    IFLT r1, r0, error # Testing <
    IFEQ r0, r1, error # Testing ==
    IFNE r0, r0, error # Testing !=
    IFLE r1, r0, error # Testing <=
    RETURN
error:
    LOAD r0, -1
    RETURN
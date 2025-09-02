## C# Coding Standards

To make our transpiler work, we must be very disciplined in how we write C# code.  This means both limiting what C# features we use, and standardize the way in which we write it.  Here we attempt to document all the restrictions.

## No shorthand integer types

Use the type names from `System` instead of the shorthand types.  For example, instead of `int`, write `Int32`.  (These compile to the same thing in C# but are more explicit.)

Including non-integer types, the full set of types you can use are: `Byte`, `SByte`, `Int16`, `UInt16`, `Int32`, `UInt32`, `Int64`, `UInt64`, `Char`, `Single`, `Double`, `Boolean`, `String`, `List<>`.

## Brace placement

- Always put an *opening brace at the end of a line*, never on a line by itself.

- Put a *close brace at the start of a line*.  It will usually be the *only* thing on that line, except for something like `} else {` or in a `switch` statement, `} break;`.


## To be continued...

We'll add more restrictions here as we remember/implement them.


## Indentation

The transpiler may rely on indentation to disambiguate code where it would otherwise take a full C# parser to do so.  So, it is important to follow these indentation rules:

1. **Indent with one tab per level.**  No indenting with spaces.  Ever.
2. **Don't indent for the `namespace` block.**  Currently we target C# 9, so we can't use the file-scoped namespace feature of C# 10, so we follow the (common) style of not indenting within the `namespace Foo { ... }` block.

The result of these rules is that a class, struct, or enum declaration will always be flush-left (not indented), and their members will be indented with exactly one tab.

Example:
```csharp
namespace MyNamespace {

public class MyClass {
	public int value;

	public void DoSomething() {
		value = 42;
	}
}

}
```

## Brace placement

- Always put an **opening brace at the end of a line**, never on a line by itself.

- Put a **close brace at the start of a line**.  It will usually be the *only* thing on that line, except for something like `} else {` or in a `switch` statement, `} break;`.


## Inline Methods

To make a method inline, add these `using` clauses at the top of your .cs file:
```
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.MethodImplOptions;
```
...and then decorate your method with `[MethodImpl(AggressiveInlining)]`.

Example:
```
using System.Runtime.CompilerServices;
using static System.Runtime.CompilerServices.MethodImplOptions;

public struct Token {
	public TokenType type;
	public String text;

	[MethodImpl(AggressiveInlining)]
	public Token(TokenType type, String text = null) {
		this.type = type;
		this.text = text;
	}
}
```

This will cue the transpiler to put the method body in the header, rather than in the .cpp file.

## Data Types (Basic)

Use the type names from `System` instead of the shorthand types.  For example, instead of `int`, write `Int32`.  (These compile to the same thing in C# but are more explicit.)

Including non-integer types, the full set of types you can use are: `Byte`, `SByte`, `Int16`, `UInt16`, `Int32`, `UInt32`, `Int64`, `UInt64`, `Char`, `Single`, `Double`, `Boolean`, `String`, `List<>`.

## Data Types (Class/Struct/Etc.)

Consider carefully what you use to encapsulate functions and data.  Rules of thumb:

- Use `struct` for anything small enough to be passed around on stack and safely copied.  These become `struct` values (stack-based, copied) in C++.
- Use `class` for larger chunks of data, or when you need reference semantics.  These become a pair of classes in C++: a storage class and a `shared_ptr` wrapper.  These use reference-counting, so avoid reference cycles, or be careful to break them!
- Use `static class` when you need no data at all, but just need to collect together a bunch of static methods or constants.
- The only generic classes allowed currently are `List` and `Dictionary`.

Here's the breakdown of how various C# concepts transpile to C++ (or not):
- class  -->  class (with our smart_ptr wrapper approach)
- struct  -->  struct
- enum  -->  enum class
- interface  -->  struct (with only abstract methods)
- delegate: disallowed (or handled as a special case)
- partial: disallowed
- record: disallowed
- custom generics: disallowed


## No indented blocks without braces

Never, _ever_ do something like this:

```
        if (Value.Equal(_items[i], item)) 
            return i;
```

Nor this:

```
        for (int i = 0; i < s.Length; i++)
            parts[i] = s[i].ToString();
```

This is evil and dangerous in any C-derived language.  Either put it all on one line, or use proper curly braces.  Do this:

```
        if (Value.Equal(_items[i], item)) return i;
```

Or do this:

```
        for (int i = 0; i < s.Length; i++) {
            parts[i] = s[i].ToString();
        }
```


## Other limitations

- The `switch` statement may only be used with integer types (including enums).  For Strings or other custom types, use `if` statements instead.

## Capitalization

While not strictly required for the transpiler, in this project we follow C# capitalization conventions:

- All class names use PascalCase
- All public members (fields, methods, properties) also use PascalCase
- Private/protected fields use _underscoreCamelCase
- Parameters and local variables use camelCase

Example:
```
public class MySuperClass {
    public Int32 MyIntField;
    private String _secretStuff;
    
    public void DoThings(Int32 withValue, Boolean quickly) {
    	Int64 temp = withValue * 7;
    	if (!quickly) Utils::DoSlowThing();
    	DoQuickThing(temp);
    }
}
```

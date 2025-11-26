
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

## Brace Placement

Braces should always go at the end of the line, not on a line by themselves.


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

## Overview

In C#, variables that reference a class instance are always _references_, and they're garbage collected, so we don't worry about who owns them, what their lifetime might be, or whether we are getting a unique copy when we do an assignment or pass one into a method.   In C++, more care is needed; an object can live on the stack or in the heap, and if passed naively, it is copied rather than referenced.  We often want to pass objects by reference rather than by value; and when storing for longer-term use, we often want to keep a pointer to the object, rather than a copy of it.

To enable this, we're making use of `using` to create aliases in C# that tell the transpiler whether to transpile that type as a pointer, or as a plain (stack) type.  It looks like this:

```cs
using ParserPtr = Parser; // CPP: using ParserPtr = Parser*;
```

This line, located at the top of the file with the other `using` statements, tells C# that `ParserPtr` is just another word for `Parser`.  But in C++, this becomes

```cpp
using ParserPtr = Parser*;
```
which tells C++ that `ParserPtr` is another word for `Parser*`, i.e. a _pointer_ to a Parser object.  The transpiler is aware of this convention, so that if `someParser` is a `ParserPtr`, it will change `someParser.thing` into `someParser->thing`.

## Reference Parameters

We do a similar trick to tell the parser when to compile a parameter as a reference parameter (rather than a copy).  At the top of the file, put

```cs
using ParserRef = Parser;
```

and then where you declare a function, use that as the parameter type:

```cs
void ParseWith(ParserRef parser) {
```

This again has no effect on the C# code, but it tells the transpiler that this should become

```cpp
void ParseWith(Parser& parser) {
```

(TODO: can we skip the transpiler magic by simply putting the same `using` clause in the C++ code?  That might be worth doing.)

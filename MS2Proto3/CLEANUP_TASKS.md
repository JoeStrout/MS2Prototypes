- We need simple, consistent conversion functions between String and Value.  See Assembler.cs:198 or Assembler.cs:1090 for example.

- C# strings are indexed by character (bytes are not accessible).  C++ indexes by byte.  In many cases, bytes are good enough and more efficient, so I don't want to just always index by character.  But maybe we can unify this by a set of extension methods for C#, and corresponding methods for our C++ string, so that the syntax is the same.  As an example, IsStringLiteral (Assembler.cs:1054) could be such a method.

- Declaration of char arrays is something the transpiler should handle:
		char[] hexChars = new char[8]; // CPP: char hexChars[9]; hexChars[8] = 0;
There are several examples like this in StringUtils.cs.

- StringUtils.makeRepr is (in C++) calling through to another makeRepr method with pool 0... this seems very sus.

- The fact that a FuncDef can be null in C#, but not in C++, is a frequent source of specialized code.  Also, we often use FuncDef& in C++ to avoid copying the whole struct.  Maybe FuncDef in C++ should actually be a pointer (or a tiny struct that contains and calls through to a pointer)?  This would require some extra code to deallocate them when done, or maybe store the actual definitions in a MemPool.  Needs careful thought.

- Can we somehow clean up this mess in VM.cs:
	Span<Value> localStack = CollectionsMarshal.AsSpan(stack).Slice(baseIndex); // CPP: Value* localStack = stackPtr + baseIndex;
We could make a list extension that reduces this to
	Span<Value> localStack = stack.AsSpanFrom(baseIndex);
and then maybe have a similar method in C++.  And maybe the transpiler could convert Span<Value> to Value*, and we'd no longer need the CPP switch.

- In our main dispatch loop in VM.cs, every case be like:
		case Opcode.LOAD_rA_kBC: { // CPP: VM_CASE(LOAD_rA_kBC) {
This is so regular, we should probably have the transpiler do it for us.  Maybe have it also set a flag and translate the subsequent `break;` statement into `VM_NEXT();`.

- There are places where a CPP switch is used just to change a type to a reference, e.g. VM.cs:471:
	CallInfo frame = callStack[callStackTop]; // CPP: CallInfo& frame = callStack[callStackTop];
Similar things occur with FuncRef elsewhere.  We could automate these by putting `using CallInfoRef = CallInfo;` at the top of the file, and then having the transpiler recognize these "Ref" type names and change it to `&`.  If we commit to C# 10 or later (Unity 2022 or later), we could even use `global using` to define these aliases project-wide (put this in GlobalUsings.cs).


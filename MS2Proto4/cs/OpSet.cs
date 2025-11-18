// OpSet: a parser operation set, i.e., a collection of function references
// that tell a parser what to do when it encounters various sorts of operations
// in the course of parsing code.
//
// Memory: This class contains a dozen or so function references, each of which
// is probably 8-16 bytes, depending on the system, and so is too big to be
// comfortably copied everywhere.  It does not use managed memory (e.g. MemPool).
// So, it should be owned by something and deleted when no longer needed, and
// passed around by reference or pointer.
//
// In typical usage, a method might allocate one of these on the stack, use it
// to configure a Parser, and then allow it to go out of scope -- this is fine.

using System;
// CPP: #include <functional>

namespace MS2Proto4 {

// Delegate types for different operation signatures.
// These act like function pointers, allowing parselets to store
// and invoke specific operations without knowing their implementation.

public delegate Double UnaryOpFunc(Double x);
public delegate Double BinaryOpFunc(Double a, Double b);
public delegate Double VarGetFunc(String identifier);
public delegate Double VarSetFunc(String identifier, Double value);
public delegate Double CallFunc(String identifier, Double arg);
public delegate Double NumValFunc(String s);

// OpSet: a collection of operation delegates that define what the parser does.
// Different OpSet implementations can make the parser do different things:
// - Evaluate expressions immediately (EvalOpSet)
// - Build an abstract syntax tree (ASTOpSet)
// - Generate bytecode (BytecodeOpSet)
// - Perform syntax coloring (ColorOpSet)
public class OpSet {
	public NumValFunc numVal;
	public VarGetFunc getVar;
	public VarSetFunc setVar;
	public CallFunc call;
	public UnaryOpFunc unaryMinus;
	public BinaryOpFunc add;
	public BinaryOpFunc subtract;
	public BinaryOpFunc multiply;
	public BinaryOpFunc divide;
	public BinaryOpFunc mod;
	public BinaryOpFunc power;
	public UnaryOpFunc factorial;
}

}

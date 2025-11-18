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

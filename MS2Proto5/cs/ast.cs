using System;
using System.Collections.Generic;

// Type alias for C++: In C#, ASTNodeUPtr is just ASTNode.
// The transpiler will generate: using ASTNodeUPtr = std::unique_ptr<ASTNode>;
using ASTNodeUPtr = MS2Proto5.ASTNode;

namespace MS2Proto5 {

// Operator constants (stored as strings, at least for now, to ease debugging).
  public static class Op {
      public const String PLUS = "PLUS";
      public const String MINUS = "MINUS";
      public const String TIMES = "TIMES";
      public const String DIVIDE = "DIVIDE";
      public const String POWER = "POWER";
      public const String FACTORIAL = "FACTORIAL";
      public const String EQUALS = "EQUALS";
      public const String NOT_EQUALS = "NOT_EQUAL";
      public const String LESS_THAN = "LESS_THAN";
      public const String GREATER_THAN = "GREATER_THAN";
      public const String LESS_EQUAL = "LESS_EQUAL";
      public const String GREATER_EQUAL = "GREATER_EQUAL";
      public const String AND = "AND";
      public const String OR = "OR";
      public const String NOT = "NOT";
  }

// Base class for all AST nodes.
// In C++, these will be managed with unique_ptr for automatic memory management.
public abstract class ASTNode {
	// Each node type should override this to provide a string representation
	public abstract new String ToString();

	// Simplify this node (constant folding and other optimizations)
	// Returns a simplified version of this node (may be a new node, or this node unchanged)
	public abstract ASTNodeUPtr Simplify();
}

// Number literal node (e.g., 42, 3.14)
public class NumberNode : ASTNode {
	public Double value;

	public NumberNode(Double value) {
		this.value = value;
	}

	public override String ToString() {
		return value.ToString();
	}

	public override ASTNodeUPtr Simplify() {
		return this;  // Already as simple as it gets
	}
}

// Identifier node (e.g., variable name like "x" or "foo")
public class IdentifierNode : ASTNode {
	public String name;

	public IdentifierNode(String name) {
		this.name = name;
	}

	public override String ToString() {
		return name;
	}

	public override ASTNodeUPtr Simplify() {
		return this;  // Can't simplify a variable reference
	}
}

// Assignment node (e.g., x = 42, foo = bar + 1)
public class AssignmentNode : ASTNode {
	public String variable;      // variable name being assigned to
	public ASTNodeUPtr value;    // expression being assigned

	public AssignmentNode(String variable, ASTNodeUPtr value) {
		this.variable = variable;
		this.value = value;
	}

	public override String ToString() {
		return variable + " = " + value.ToString();
	}

	public override ASTNodeUPtr Simplify() {
		ASTNodeUPtr simplifiedValue = value.Simplify();
		return new AssignmentNode(variable, simplifiedValue);
	}
}

// Unary operator node (e.g., -x, !flag)
public class UnaryOpNode : ASTNode {
	public String op;          // Op.MINUS or Op.FACTORIAL
	public ASTNodeUPtr operand; // the expression being operated on

	public UnaryOpNode(String op, ASTNodeUPtr operand) {
		this.op = op;
		this.operand = operand;
	}

	public override String ToString() {
		return op + "(" + operand.ToString() + ")";
	}

	public override ASTNodeUPtr Simplify() {
		ASTNodeUPtr simplifiedOperand = operand.Simplify();

		// If operand is a constant, compute the result
		if (simplifiedOperand is NumberNode) {
			NumberNode num = (NumberNode)simplifiedOperand;
			if (op == Op.MINUS) {
				return new NumberNode(-num.value);
			} else if (op == Op.FACTORIAL) {
				// Compute factorial
				Double result = 1;
				for (Int32 i = 2; i <= (Int32)num.value; i++) {
					result *= i;
				}
				return new NumberNode(result);
			}
		}

		// Otherwise return unary op with simplified operand
		return new UnaryOpNode(op, simplifiedOperand);
	}
}

// Binary operator node (e.g., x + y, a * b)
public class BinaryOpNode : ASTNode {
	public String op;           // Op.PLUS, etc.
	public ASTNodeUPtr left;    // left operand
	public ASTNodeUPtr right;   // right operand

	public BinaryOpNode(String op, ASTNodeUPtr left, ASTNodeUPtr right) {
		this.op = op;
		this.left = left;
		this.right = right;
	}

	public override String ToString() {
		return op + "(" + left.ToString() + ", " + right.ToString() + ")";
	}

	public override ASTNodeUPtr Simplify() {
		ASTNodeUPtr simplifiedLeft = left.Simplify();
		ASTNodeUPtr simplifiedRight = right.Simplify();

		// If both operands are constants, compute the result
		if (simplifiedLeft is NumberNode && simplifiedRight is NumberNode) {
			NumberNode leftNum = (NumberNode)simplifiedLeft;
			NumberNode rightNum = (NumberNode)simplifiedRight;

			if (op == Op.PLUS) {
				return new NumberNode(leftNum.value + rightNum.value);
			} else if (op == Op.MINUS) {
				return new NumberNode(leftNum.value - rightNum.value);
			} else if (op == Op.TIMES) {
				return new NumberNode(leftNum.value * rightNum.value);
			} else if (op == Op.DIVIDE) {
				return new NumberNode(leftNum.value / rightNum.value);
			} else if (op == Op.POWER) {
				return new NumberNode(Math.Pow(leftNum.value, rightNum.value));
			}
		}

		// Otherwise return binary op with simplified operands
		return new BinaryOpNode(op, simplifiedLeft, simplifiedRight);
	}
}

// Function call node (e.g., sqrt(x), max(a, b))
public class CallNode : ASTNode {
	public String function;               // function name
	public List<ASTNodeUPtr> arguments;  // list of argument expressions

	public CallNode(String function, List<ASTNodeUPtr> arguments) {
		this.function = function;
		this.arguments = arguments;
	}

	public CallNode(String function) {
		this.function = function;
		this.arguments = new List<ASTNodeUPtr>();
	}

	public override String ToString() {
		String result = function + "(";
		for (Int32 i = 0; i < arguments.Count; i++) {
			if (i > 0) result += ", ";
			result += arguments[i].ToString();
		}
		result += ")";
		return result;
	}

	public override ASTNodeUPtr Simplify() {
		List<ASTNodeUPtr> simplifiedArgs = new List<ASTNodeUPtr>();
		for (Int32 i = 0; i < arguments.Count; i++) {
			simplifiedArgs.Add(arguments[i].Simplify());
		}
		return new CallNode(function, simplifiedArgs);
	}
}

// Grouping node (e.g., parenthesized expression like "(x + y)")
// This may not be strictly necessary for evaluation, but can be useful
// for preserving structure for pretty-printing or code generation.
public class GroupNode : ASTNode {
	public ASTNodeUPtr expression;

	public GroupNode(ASTNodeUPtr expression) {
		this.expression = expression;
	}

	public override String ToString() {
		return "(" + expression.ToString() + ")";
	}

	public override ASTNodeUPtr Simplify() {
		// Groups don't affect value, just return simplified child
		return expression.Simplify();
	}
}

} // namespace MS2Proto5

using System;
using System.Collections.Generic;
// CPP: #include "CS_Math.h"

using ASTNodeSPtr = MS2Proto5.ASTNode;

namespace MS2Proto5 {

// CPP: class ASTNode;
// CPP: using ASTNodeSPtr = std::shared_ptr<ASTNode>;

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
public abstract class ASTNode { // CPP: class ASTNode : public std::enable_shared_from_this<ASTNode> {
	// CPP: public: virtual ~ASTNode() {}

	// Each node type should override this to provide a string representation
	public abstract new String ToString();  // CPP: public: virtual String ToString() = 0;

	// Simplify this node (constant folding and other optimizations)
	// Returns a simplified version of this node (may be a new node, or this node unchanged)
	public abstract ASTNodeSPtr Simplify();
} // CPP: };

// Number literal node (e.g., 42, 3.14)
public class NumberNode : ASTNode {
	public Double value;

	public NumberNode(Double value) {
		this.value = value;
	}

	public override String ToString() {
		return StringExtras.ToString(value); // CPP: return ::ToString(value); // Grr! Argh!
	}

	public override ASTNodeSPtr Simplify() {
		return this;  // CPP: return shared_from_this();
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

	public override ASTNodeSPtr Simplify() {
		return this;  // CPP: return shared_from_this();
	}
}

// Assignment node (e.g., x = 42, foo = bar + 1)
public class AssignmentNode : ASTNode {
	public String variable;      // variable name being assigned to
	public ASTNodeSPtr value;    // expression being assigned

	public AssignmentNode(String variable, ASTNodeSPtr value) {
		this.variable = variable;
		this.value = value;
	}

	public override String ToString() {
		return variable + " = " + value.ToString();
	}

	public override ASTNodeSPtr Simplify() {
		ASTNodeSPtr simplifiedValue = value.Simplify();
		return new AssignmentNode(variable, simplifiedValue); // CPP: return std::make_shared<AssignmentNode>(variable, simplifiedValue);
	}
}

// Unary operator node (e.g., -x, !flag)
public class UnaryOpNode : ASTNode {
	public String op;          // Op.MINUS or Op.FACTORIAL
	public ASTNodeSPtr operand; // the expression being operated on

	public UnaryOpNode(String op, ASTNodeSPtr operand) {
		this.op = op;
		this.operand = operand;
	}

	public override String ToString() {
		return op + "(" + operand.ToString() + ")";
	}

	public override ASTNodeSPtr Simplify() {
		ASTNodeSPtr simplifiedOperand = operand.Simplify();

		// If operand is a constant, compute the result
		NumberNode num = simplifiedOperand as NumberNode;
		if (num != null) {
			if (op == Op.MINUS) {
				return new NumberNode(-num.value); // CPP: return std::make_shared<NumberNode>(-num->value);
			} else if (op == Op.FACTORIAL) {
				// Compute factorial
				Double result = 1;
				for (Int32 i = 2; i <= (Int32)num.value; i++) {
					result *= i;
				}
				return new NumberNode(result); // CPP: return std::make_shared<NumberNode>(result);
			}
		}

		// Otherwise return unary op with simplified operand
		return new UnaryOpNode(op, simplifiedOperand); // CPP: return std::make_shared<UnaryOpNode>(op, simplifiedOperand);
	}
}

// Binary operator node (e.g., x + y, a * b)
public class BinaryOpNode : ASTNode {
	public String op;           // Op.PLUS, etc.
	public ASTNodeSPtr left;    // left operand
	public ASTNodeSPtr right;   // right operand

	public BinaryOpNode(String op, ASTNodeSPtr left, ASTNodeSPtr right) {
		this.op = op;
		this.left = left;
		this.right = right;
	}

	public override String ToString() {
		return op + "(" + left.ToString() + ", " + right.ToString() + ")";
	}

	public override ASTNodeSPtr Simplify() {
		ASTNodeSPtr simplifiedLeft = left.Simplify();
		ASTNodeSPtr simplifiedRight = right.Simplify();

		// If both operands are constants, compute the result
		var leftNum = simplifiedLeft as NumberNode;
		var rightNum = simplifiedRight as NumberNode;
		if (leftNum != null && rightNum != null) {
			if (op == Op.PLUS) {
				return new NumberNode(leftNum.value + rightNum.value); // CPP: return std::make_shared<NumberNode>(leftNum->value + rightNum->value);
			} else if (op == Op.MINUS) {
				return new NumberNode(leftNum.value - rightNum.value); // CPP: return std::make_shared<NumberNode>(leftNum->value - rightNum->value);
			} else if (op == Op.TIMES) {
				return new NumberNode(leftNum.value * rightNum.value); // CPP: return std::make_shared<NumberNode>(leftNum->value * rightNum->value);
			} else if (op == Op.DIVIDE) {
				return new NumberNode(leftNum.value / rightNum.value); // CPP: return std::make_shared<NumberNode>(leftNum->value / rightNum->value);
			} else if (op == Op.POWER) {
				return new NumberNode(Math.Pow(leftNum.value, rightNum.value)); // CPP: return std::make_shared<NumberNode>(Math::Pow(leftNum->value, rightNum->value));
			}
		}

		// Otherwise return binary op with simplified operands
		return new BinaryOpNode(op, simplifiedLeft, simplifiedRight); // CPP: return std::make_shared<BinaryOpNode>(op, simplifiedLeft, simplifiedRight);
	}
}

// Function call node (e.g., sqrt(x), max(a, b))
public class CallNode : ASTNode {
	public String function;               // function name
	public List<ASTNodeSPtr> arguments;  // list of argument expressions

	public CallNode(String function, List<ASTNodeSPtr> arguments) {
		this.function = function;
		this.arguments = arguments;
	}

	public CallNode(String function) {
		this.function = function;
		this.arguments = new List<ASTNodeSPtr>();
	}

	public override String ToString() {
		String result = function + "(";
		for (Int32 i = 0; i < arguments.Count; i++) {
			if (i > 0) result += ", ";
			result += arguments[i].ToString(); // CPP: result += arguments[i]->ToString(); // ToDo: fix transpiler!
		}
		result += ")";
		return result;
	}

	public override ASTNodeSPtr Simplify() {
		List<ASTNodeSPtr> simplifiedArgs = new List<ASTNodeSPtr>();
		for (Int32 i = 0; i < arguments.Count; i++) {
			simplifiedArgs.Add(arguments[i].Simplify()); // CPP: simplifiedArgs.Add(arguments[i]->Simplify()); // ToDo: fix transpiler!
		}
		return new CallNode(function, simplifiedArgs); // CPP: return std::make_shared<CallNode>(function, simplifiedArgs);
	}
}

// Grouping node (e.g., parenthesized expression like "(x + y)")
// This may not be strictly necessary for evaluation, but can be useful
// for preserving structure for pretty-printing or code generation.
public class GroupNode : ASTNode {
	public ASTNodeSPtr expression;

	public GroupNode(ASTNodeSPtr expression) {
		this.expression = expression;
	}

	public override String ToString() {
		return "(" + expression.ToString() + ")";
	}

	public override ASTNodeSPtr Simplify() {
		// Groups don't affect value, just return simplified child
		return expression.Simplify();
	}
}

} // namespace MS2Proto5

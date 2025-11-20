using System;
using System.Collections.Generic;
// CPP: #include "ast.g.h"
// CPP: #include "Token.g.h"
/*** BEGIN CPP_ONLY ***
#include "Parser.g.h"
#include <iostream>
#include <stdlib.h>
*** END CPP_ONLY ***/

// Type alias for C++: In C#, ASTNodeSPtr is just ASTNode.
// The transpiler will generate: using ASTNodeSPtr = std::unique_ptr<ASTNode>;
using ASTNodeSPtr = MS2Proto5.ASTNode;

namespace MS2Proto5 {

// Parselets: mini-parsers, each responsible for parsing one
// certain type of thing. They build AST nodes instead of
// directly evaluating expressions.

// CPP: // forward declarations
// CPP: class Parser;
// CPP: class Parselet;
// CPP: class PrefixParselet;
// CPP: class InfixParselet;
// CPP: enum class Precedence : int32_t;

using ParserPtr = Parser; // CPP: using ParserPtr = Parser*;

public abstract class Parselet {
	public Precedence precedence;
	// CPP: public: virtual ~Parselet() {}
}

// PrefixParselet: abstract base for parselets that handle tokens
// starting an expression (numbers, identifiers, unary operators).
public abstract class PrefixParselet : Parselet {
	public abstract ASTNodeSPtr Parse(ParserPtr parser, List<Token> tokens);
}

// NumberParselet: handles number literals.
public class NumberParselet : PrefixParselet {

	public NumberParselet() {
	}

	public override ASTNodeSPtr Parse(ParserPtr, List<Token> tokens) {
		Token token = tokens[0];
		tokens.RemoveAt(0);
		Double value = Double.Parse(token.text); // CPP: Double value = atof(token.text.c_str());
		return new NumberNode(value); // CPP: return std::make_shared<NumberNode>(value);
	}
}

// UnaryOpParselet: handles prefix unary operators like '-' in '-x'.
public class UnaryOpParselet : PrefixParselet {
	public String op;

	public UnaryOpParselet(String op, Precedence prec) {
		this.op = op;
		precedence = prec;
	}

	public override ASTNodeSPtr Parse(ParserPtr parser, List<Token> tokens) {
		tokens.RemoveAt(0);  // skip operator token
		ASTNodeSPtr operand = parser.Parse(tokens, precedence);
		return new UnaryOpNode(op, operand); // CPP: return std::make_shared<UnaryOpNode>(op, operand);
	}
}

// InfixParselet: abstract base for parselets that handle infix operators.
public abstract class InfixParselet : Parselet {
	public abstract ASTNodeSPtr Parse(ParserPtr parser, ASTNodeSPtr lhs, List<Token> tokens);
}

// BinaryOpParselet: handles binary operators like '+', '-', '*', etc.
public class BinaryOpParselet : InfixParselet {
	public String op;
	public Boolean rightAssoc = false;

	public BinaryOpParselet(String op, Precedence prec, Boolean rightAssoc = false) {
		this.op = op;
		precedence = prec;
		this.rightAssoc = rightAssoc;
	}

	public override ASTNodeSPtr Parse(ParserPtr parser, ASTNodeSPtr lhs, List<Token> tokens) {
		Precedence rhsPrec = precedence - (rightAssoc ? 1 : 0);
		ASTNodeSPtr rhs = parser.Parse(tokens, rhsPrec);
		return new BinaryOpNode(op, lhs, rhs); // CPP: return std::make_shared<BinaryOpNode>(op, lhs, rhs);
	}
}

// PostfixParselet: handles postfix unary operators like '!' for factorial.
public class PostfixParselet : InfixParselet {
	public String op;

	public PostfixParselet(String op, Precedence prec) {
		this.op = op;
		precedence = prec;
	}

	public override ASTNodeSPtr Parse(ParserPtr, ASTNodeSPtr lhs, List<Token>) {
		// This is a right-side unary operator, not actually a binary operator.
		// So we don't touch the given tokens, but instead just operate on lhs.
		return new UnaryOpNode(op, lhs); // CPP: return std::make_shared<UnaryOpNode>(op, lhs);
	}
}

// IdentifierParselet: handles identifiers, which can be:
// - Variable lookups
// - Variable assignments (when followed by '=')
// - Function calls (when followed by '(')
public class IdentifierParselet : PrefixParselet {

	public IdentifierParselet() {
	}

	public override ASTNodeSPtr Parse(ParserPtr parser, List<Token> tokens) {
		String identifier = tokens[0].text;
		tokens.RemoveAt(0);

		// Check what comes next
		TokenType nextType = tokens.Count > 0 ? tokens[0].type : TokenType.NUMBER;

		if (nextType == TokenType.ASSIGN) {
			tokens.RemoveAt(0);  // discard "="
			ASTNodeSPtr rhs = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
			return new AssignmentNode(identifier, rhs); // CPP: return std::make_shared<AssignmentNode>(identifier, rhs);
		} else if (nextType == TokenType.LPAREN) {
			tokens.RemoveAt(0);  // discard "("
			List<ASTNodeSPtr> args = new List<ASTNodeSPtr>();
			if (tokens.Count > 0 && tokens[0].type != TokenType.RPAREN) {
				args.Add(parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT));
			}
			if (tokens.Count == 0 || tokens[0].type != TokenType.RPAREN) {
				Console.WriteLine("Unbalanced parentheses"); // CPP: std::cout << "Unbalanced parentheses\n";
				return new NumberNode(0.0); // CPP: return std::make_shared<NumberNode>(0.0);
			}
			tokens.RemoveAt(0);  // discard ")"
			return new CallNode(identifier, args); // CPP: return std::make_shared<CallNode>(identifier, args);
		} else {
			return new IdentifierNode(identifier); // CPP: return std::make_shared<IdentifierNode>(identifier);
		}
	}
}

// GroupParselet: handles parenthesized expressions like '(2 + 3)'.
public class GroupParselet : PrefixParselet {

	public GroupParselet() {
	}

	public override ASTNodeSPtr Parse(ParserPtr parser, List<Token> tokens) {
		tokens.RemoveAt(0);  // discard "("
		ASTNodeSPtr result = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
		if (tokens.Count == 0 || tokens[0].type != TokenType.RPAREN) {
			Console.WriteLine("Unbalanced parentheses"); // CPP: std::cout << "Unbalanced parentheses\n";
			return new NumberNode(0.0); // CPP: return std::make_shared<NumberNode>(0.0);
		}
		tokens.RemoveAt(0);  // discard ")"
		return new GroupNode(result); // CPP: return std::make_shared<GroupNode>(result);
	}
}

}

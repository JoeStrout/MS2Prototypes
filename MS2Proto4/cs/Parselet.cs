using System;
using System.Collections.Generic;
// CPP: #include "OpSet.g.h"
// CPP: #include "Token.g.h"
/*** BEGIN CPP_ONLY ***
#include "Parser.g.h"
#include <iostream>
*** END CPP_ONLY ***/

namespace MS2Proto4 {

// Parselets: mini-parsers, each responsible for parsing one
// certain type of thing. Some of them are invoked by only
// one token; others might be invoked by many tokens, and
// parameterized to control what they do.

// CPP: // forward declarations
// CPP: class Parser;
// CPP: class Parselet;
// CPP: class PrefixParselet;
// CPP: class InfixParselet;
// CPP: enum class Precedence : int32_t;

using ParserPtr = Parser; // CPP: using ParserPtr = Parser*;

public abstract class Parselet {
	public Precedence precedence;
}

// PrefixParselet: abstract base for parselets that handle tokens
// starting an expression (numbers, identifiers, unary operators).
public abstract class PrefixParselet : Parselet {
	public abstract Double Parse(ParserPtr parser, List<Token> tokens);
}

// NumberParselet: handles number literals.
public class NumberParselet : PrefixParselet {
	public NumValFunc operation;

	public NumberParselet(NumValFunc op) {
		operation = op;
	}

	public override Double Parse(ParserPtr parser, List<Token> tokens) {
		Token token = tokens[0];
		tokens.RemoveAt(0);
		return operation(token.text);
	}
}

// UnaryOpParselet: handles prefix unary operators like '-' in '-x'.
public class UnaryOpParselet : PrefixParselet {
	public UnaryOpFunc operation;

	public UnaryOpParselet(UnaryOpFunc op, Precedence prec) {
		operation = op;
		precedence = prec;
	}

	public override Double Parse(ParserPtr parser, List<Token> tokens) {
		tokens.RemoveAt(0);  // skip operator token
		Double operand = parser.Parse(tokens, precedence);
		return operation(operand);
	}
}

// InfixParselet: abstract base for parselets that handle infix operators.
public abstract class InfixParselet : Parselet {
	// in CPP, this would be public: virtual Double Parse(ParserPtr parser, Double lhs, List<Token> tokens) = 0;
	public abstract Double Parse(ParserPtr parser, Double lhs, List<Token> tokens);
}

// BinaryOpParselet: handles binary operators like '+', '-', '*', etc.
public class BinaryOpParselet : InfixParselet {
	public BinaryOpFunc operation;
	public Boolean rightAssoc = false;

	public BinaryOpParselet(BinaryOpFunc op, Precedence prec, Boolean rightAssoc = false) {
		operation = op;
		precedence = prec;
		this.rightAssoc = rightAssoc;
	}

	public override Double Parse(ParserPtr parser, Double lhs, List<Token> tokens) {
		Precedence rhsPrec = precedence - (rightAssoc ? 1 : 0);
		Double rhs = parser.Parse(tokens, rhsPrec);
		return operation(lhs, rhs);
	}
}

// PostfixParselet: handles postfix unary operators like '!' for factorial.
public class PostfixParselet : InfixParselet {
	public UnaryOpFunc operation;

	public PostfixParselet(UnaryOpFunc op, Precedence prec) {
		operation = op;
		precedence = prec;
	}

	public override Double Parse(ParserPtr parser, Double lhs, List<Token> tokens) {
		// This is a right-side unary operator, not actually a binary operator.
		// So we don't touch the given tokens, but instead just operate on lhs.
		return operation(lhs);
	}
}

// IdentifierParselet: handles identifiers, which can be:
// - Variable lookups
// - Variable assignments (when followed by '=')
// - Function calls (when followed by '(')
public class IdentifierParselet : PrefixParselet {
	public VarGetFunc operation;
	public VarSetFunc assignmentOp;
	public CallFunc callOp;

	public IdentifierParselet(VarGetFunc getOp, VarSetFunc setOp, CallFunc callOp) {
		operation = getOp;
		assignmentOp = setOp;
		this.callOp = callOp;
	}

	public override Double Parse(ParserPtr parser, List<Token> tokens) {
		String identifier = tokens[0].text;
		tokens.RemoveAt(0);

		// Check what comes next
		TokenType nextType = tokens.Count > 0 ? tokens[0].type : TokenType.NUMBER;

		if (nextType == TokenType.ASSIGN) {
			tokens.RemoveAt(0);  // discard "="
			Double rhs = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
			return assignmentOp(identifier, rhs);
		} else if (nextType == TokenType.LPAREN) {
			tokens.RemoveAt(0);  // discard "("
			Double arg = 0.0;
			if (tokens.Count > 0 && tokens[0].type != TokenType.RPAREN) {
				arg = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
			}
			if (tokens.Count == 0 || tokens[0].type != TokenType.RPAREN) {
				Console.WriteLine("Unbalanced parentheses"); // CPP: std::cout << "Unbalanced parentheses\n";
				return 0.0;
			}
			tokens.RemoveAt(0);  // discard ")"
			return callOp(identifier, arg);
		} else {
			return operation(identifier);
		}
	}
}

// GroupParselet: handles parenthesized expressions like '(2 + 3)'.
public class GroupParselet : PrefixParselet {

	public GroupParselet() {
	}

	public override Double Parse(ParserPtr parser, List<Token> tokens) {
		tokens.RemoveAt(0);  // discard "("
		Double result = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
		if (tokens.Count == 0 || tokens[0].type != TokenType.RPAREN) {
			Console.WriteLine("Unbalanced parentheses"); // CPP: std::cout << "Unbalanced parentheses\n";
			return 0.0;
		}
		tokens.RemoveAt(0);  // discard ")"
		return result;
	}
}

}

using System;
using System.Collections.Generic;
// CPP: #include "OpSet.g.h"
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
// CPP: enum class Precedence : int32_t;

using ParserPtr = Parser; // CPP: using ParserPtr = Parser*;

public class Parselet {
	public Precedence precedence;
}

// PrefixParselet: handles tokens that start an expression,
// like numbers, identifiers, or unary operators.
public class PrefixParselet : Parselet {
	public NumValFunc operation;

	public PrefixParselet() {
		operation = null;
	}

	public PrefixParselet(NumValFunc op) {
		operation = op;
	}

	public virtual Double Parse(List<String> tokens) {
		String token = tokens[0];
		tokens.RemoveAt(0);
		return operation(token);
	}
}

// UnaryOpParselet: handles prefix unary operators like '-' in '-x'.
public class UnaryOpParselet : PrefixParselet {
	public new UnaryOpFunc operation;
	public ParserPtr parser;

	public UnaryOpParselet(UnaryOpFunc op, Precedence prec, ParserPtr parser) {
		operation = op;
		precedence = prec;
		this.parser = parser;
	}

	public override Double Parse(List<String> tokens) {
		tokens.RemoveAt(0);  // skip operator token
		Double operand = parser.Parse(tokens, precedence);
		return operation(operand);
	}
}

// InfixParselet: handles binary operators like '+', '-', '*', etc.
public class InfixParselet : Parselet {
	public BinaryOpFunc operation;
	public Boolean rightAssoc = false;
	public ParserPtr parser;

	public InfixParselet() {
		operation = null;
		parser = null;
	}

	public InfixParselet(BinaryOpFunc op, Precedence prec, ParserPtr parser, Boolean rightAssoc = false) {
		operation = op;
		precedence = prec;
		this.parser = parser;
		this.rightAssoc = rightAssoc;
	}

	public virtual Double Parse(Double lhs, List<String> tokens) {
		Precedence rhsPrec = precedence - (rightAssoc ? 1 : 0);
		Double rhs = parser.Parse(tokens, rhsPrec);
		return operation(lhs, rhs);
	}
}

// PostfixParselet: handles postfix unary operators like '!' for factorial.
public class PostfixParselet : InfixParselet {
	public new UnaryOpFunc operation;

	public PostfixParselet(UnaryOpFunc op, Precedence prec, ParserPtr parser) {
		operation = op;
		precedence = prec;
		this.parser = parser;
	}

	public override Double Parse(Double lhs, List<String> tokens) {
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
	public new VarGetFunc operation;
	public VarSetFunc assignmentOp;
	public CallFunc callOp;
	public ParserPtr parser;

	public IdentifierParselet(VarGetFunc getOp, VarSetFunc setOp, CallFunc callOp, ParserPtr parser) {
		operation = getOp;
		assignmentOp = setOp;
		this.callOp = callOp;
		this.parser = parser;
	}

	public override Double Parse(List<String> tokens) {
		String identifier = tokens[0];
		tokens.RemoveAt(0);

		// Check what comes next
		String nextToken = null;
		if (tokens.Count > 0) nextToken = tokens[0];

		if (nextToken == "=") {
			tokens.RemoveAt(0);  // discard "="
			Double rhs = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
			return assignmentOp(identifier, rhs);
		} else if (nextToken == "(") {
			tokens.RemoveAt(0);  // discard "("
			Double arg = 0.0;
			if (tokens.Count > 0 && tokens[0] != ")") {
				arg = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
			}
			if (tokens.Count == 0 || tokens[0] != ")") {
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
	public ParserPtr parser;

	public GroupParselet(ParserPtr parser) {
		this.parser = parser;
	}

	public override Double Parse(List<String> tokens) {
		tokens.RemoveAt(0);  // discard "("
		Double result = parser.Parse(tokens, Precedence.BELOW_ASSIGNMENT);
		if (tokens.Count == 0 || tokens[0] != ")") {
			Console.WriteLine("Unbalanced parentheses"); // CPP: std::cout << "Unbalanced parentheses\n";
			return 0.0;
		}
		tokens.RemoveAt(0);  // discard ")"
		return result;
	}
}

}

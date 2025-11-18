using System;
using System.Collections.Generic;
// CPP: #include "Parselet.g.h"
// CPP: #include "Lexer.g.h"

namespace MS2Proto4 {

using StringPrefixDict = Dictionary<String, PrefixParselet>;
using StringInfixDict = Dictionary<String, InfixParselet>;
// CPP: using StringPrefixDict = Dictionary<String, PrefixParselet>;
// CPP: using StringInfixDict = Dictionary<String, InfixParselet>;

// Precedence levels (higher precedence binds more strongly).
public enum Precedence : Int32 {
	BELOW_ASSIGNMENT = 0,
	ASSIGNMENT = 1,
	CONDITIONAL = 2,
	SUM = 3,
	PRODUCT = 4,
	EXPONENT = 5,
	FACTORIAL = 6,
	PREFIX = 7,
	POSTFIX = 8,
	CALL = 9
}

// CPP: inline Precedence operator-(Precedence p, int offset) { return (Precedence)((int)p - offset); }

// Parser: the main parsing engine.
// Uses a Pratt parser algorithm with parselets to handle operator precedence.
public class Parser {

	private StringPrefixDict _prefixParsers;
	private StringInfixDict _infixParsers;

	public Parser(OpSet ops) {
		_prefixParsers = new StringPrefixDict();
		_infixParsers = new StringInfixDict();
		BuildTokenEffects(ops);
	}

	// Build the token effects table from an OpSet
	private void BuildTokenEffects(OpSet ops) {
		// Special parselets that don't directly map to operations
		PrefixParselet numberParselet = new PrefixParselet(ops.numVal);
		IdentifierParselet identParselet = new IdentifierParselet(ops.getVar, ops.setVar, ops.call, this);
		GroupParselet groupParselet = new GroupParselet(this);

		// Tokens with prefix parselets
		_prefixParsers["("] = groupParselet;
		_prefixParsers["number"] = numberParselet;
		_prefixParsers["ident"] = identParselet;

		// Minus can be both prefix (unary) and infix (binary)
		_prefixParsers["-"] = new UnaryOpParselet(ops.unaryMinus, Precedence.PREFIX, this);
		_infixParsers["-"] = new InfixParselet(ops.subtract, Precedence.SUM, this);

		// Binary operators
		_infixParsers["+"] = new InfixParselet(ops.add, Precedence.SUM, this);
		_infixParsers["*"] = new InfixParselet(ops.multiply, Precedence.PRODUCT, this);
		_infixParsers["/"] = new InfixParselet(ops.divide, Precedence.PRODUCT, this);
		_infixParsers["%"] = new InfixParselet(ops.mod, Precedence.PRODUCT, this);
		_infixParsers["^"] = new InfixParselet(ops.power, Precedence.EXPONENT, this, true);

		// Postfix operators
		_infixParsers["!"] = new PostfixParselet(ops.factorial, Precedence.FACTORIAL, this);
	}

	// Get the precedence of the infix parser for a token
	private Precedence InfixPrecedence(String token) {
		if (!_infixParsers.ContainsKey(token)) return (Precedence)(-1);
		InfixParselet parselet = _infixParsers[token];
		return parselet.precedence;
	}

	// Parse a list of tokens and return the result
	public Double Parse(List<String> tokens, Precedence precedence) {
		if (tokens == null || tokens.Count == 0) return 0.0;

		// First, check prefix parsers (for tokens that can start an expression)
		String firstTok = tokens[0];
		PrefixParselet prefixParselet = null;

		if (Lexer.IsNumber(firstTok)) {
			prefixParselet = _prefixParsers["number"];
		} else if (Lexer.IsIdentifier(firstTok)) {
			prefixParselet = _prefixParsers["ident"];
		} else if (_prefixParsers.ContainsKey(firstTok)) {
			prefixParselet = _prefixParsers[firstTok];
		}

		Double value = 0.0;
		if (prefixParselet != null) {
			value = prefixParselet.Parse(tokens);
		} else {
			Console.WriteLine("Invalid expression start: " + firstTok);
			return 0.0;
		}

		// Then, continue applying infix parsers, until
		// we hit something of lower (or same) precedence.
		while (tokens.Count > 0 && InfixPrecedence(tokens[0]) > precedence) {
			String operatorToken = tokens[0];
			tokens.RemoveAt(0);
			if (!_infixParsers.ContainsKey(operatorToken)) break;
			InfixParselet infixParselet = _infixParsers[operatorToken];
			value = infixParselet.Parse(value, tokens);
		}

		return value;
	}

	// Convenience overload that uses default precedence
	public Double Parse(List<String> tokens) {
		return Parse(tokens, Precedence.BELOW_ASSIGNMENT);
	}
}

}

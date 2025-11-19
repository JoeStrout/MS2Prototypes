// Parser: the main parsing class. It uses a Pratt parser algorithm to build
// an Abstract Syntax Tree (AST) from a stream of tokens.
//
// Memory: a Parser is a pretty big object, containing two arrays of parselets.
// Those parselets are allocated with `new`, and owned by the Parser, which
// deletes them in its destructor. The Parser itself should be kept on the stack
// and allowed to go out of scope, or explicitly `new`d and `delete`d by the user.

using System;
using System.Collections.Generic;
// CPP: #include "ast.g.h"
// CPP: #include "Parselet.g.h"
// CPP: #include "Lexer.g.h"
// CPP: #include "Token.g.h"
// CPP: #include "IOHelper.g.h"

// Type alias for C++: In C#, ASTNodeUPtr is just ASTNode.
// The transpiler will generate: using ASTNodeUPtr = std::unique_ptr<ASTNode>;
using ASTNodeUPtr = MS2Proto5.ASTNode;

namespace MS2Proto5 {

// CPP: class Parser;
using ParserPtr = Parser; // CPP: using ParserPtr = Parser*;
using PrefixParseletUPtr = PrefixParselet; // CPP: using PrefixParseletUPtr = std::unique_ptr<PrefixParselet>;
using InfixParseletUPtr = InfixParselet; // CPP: using InfixParseletUPtr = std::unique_ptr<InfixParselet>;

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

	private PrefixParseletUPtr[] _prefixParsers;
	private InfixParseletUPtr[] _infixParsers;

	public Parser() {
		// Initialize arrays (size = number of token types)
		Int32 qty = (Int32)TokenType._QTY_TOKENS;
		_prefixParsers = new PrefixParseletUPtr[qty];
		_infixParsers = new InfixParseletUPtr[qty];

		BuildTokenEffects();
	}

	// Build the token effects table
	private void BuildTokenEffects() {
		// Tokens with prefix parselets
		_prefixParsers[(Int32)TokenType.LPAREN] = new GroupParselet();
		_prefixParsers[(Int32)TokenType.NUMBER] = new NumberParselet();
		_prefixParsers[(Int32)TokenType.IDENTIFIER] = new IdentifierParselet();

		// Minus can be both prefix (unary) and infix (binary)
		_prefixParsers[(Int32)TokenType.MINUS] = new UnaryOpParselet(Op.MINUS, Precedence.PREFIX);
		_infixParsers[(Int32)TokenType.MINUS] = new BinaryOpParselet(Op.MINUS, Precedence.SUM);

		// Binary operators
		_infixParsers[(Int32)TokenType.PLUS] = new BinaryOpParselet(Op.PLUS, Precedence.SUM);
		_infixParsers[(Int32)TokenType.STAR] = new BinaryOpParselet(Op.TIMES, Precedence.PRODUCT);
		_infixParsers[(Int32)TokenType.SLASH] = new BinaryOpParselet(Op.DIVIDE, Precedence.PRODUCT);
		_infixParsers[(Int32)TokenType.CARET] = new BinaryOpParselet(Op.POWER, Precedence.EXPONENT, true);

		// Postfix operators
		_infixParsers[(Int32)TokenType.BANG] = new PostfixParselet(Op.FACTORIAL, Precedence.FACTORIAL);
	}

	// Get the precedence of the infix parser for a token type
	private Precedence InfixPrecedence(TokenType type) {
		InfixParseletUPtr parselet = _infixParsers[(Int32)type];
		if (parselet == null) return (Precedence)(-1);
		return parselet.precedence;
	}

	// Parse a list of tokens and return the AST root node
	public ASTNodeUPtr Parse(List<Token> tokens, Precedence precedence) {
		if (tokens.Count == 0) return new NumberNode(0.0);

		// First, check prefix parsers (for tokens that can start an expression)
		Token firstTok = tokens[0];
		PrefixParseletUPtr prefixParselet = _prefixParsers[(Int32)firstTok.type];

		ASTNodeUPtr node = null;
		if (prefixParselet != null) {
			node = prefixParselet.Parse(this, tokens);
		} else {
			ReportError("Invalid expression start");
			return new NumberNode(0.0);
		}

		// Then, continue applying infix parsers, until
		// we hit something of lower (or same) precedence.
		while (tokens.Count > 0 && InfixPrecedence(tokens[0].type) > precedence) {
			TokenType operatorType = tokens[0].type;
			tokens.RemoveAt(0);
			InfixParseletUPtr infixParselet = _infixParsers[(Int32)operatorType];
			if (infixParselet == null) break;
			node = infixParselet.Parse(this, node, tokens);
		}

		return node;
	}

	// Convenience overload that uses default precedence
	public ASTNodeUPtr Parse(List<Token> tokens) {
		return Parse(tokens, Precedence.BELOW_ASSIGNMENT);
	}

	public void ReportError(String errMsg) {
		IOHelper.Print(errMsg);
	}
}

}

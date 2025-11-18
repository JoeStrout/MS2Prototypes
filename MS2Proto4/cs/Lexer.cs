using System;
using System.Collections.Generic;
// CPP: #include "Token.g.h"

namespace MS2Proto4 {

// A simple lexer, handling numbers, identifiers, and operators.
// Splits a string into a list of tokens, stripping out whitespace.
public static class Lexer {

	private static String operatorChars = "+-*/%^()=!";

	private enum State : Byte {
		START = 0,
		NUMBER = 1,
		OPERATOR = 2,
		IDENTIFIER = 3
	}

	// Convert a string token to a Token with appropriate TokenType
	private static Token MakeToken(String tokenText, State state) {
		if (state == State.NUMBER) {
			return new Token(TokenType.NUMBER, tokenText);
		} else if (state == State.IDENTIFIER) {
			return new Token(TokenType.IDENTIFIER, tokenText);
		} else if (state == State.OPERATOR) {
			// Map operator characters to token types
			if (tokenText == "+") return new Token(TokenType.PLUS);
			if (tokenText == "-") return new Token(TokenType.MINUS);
			if (tokenText == "*") return new Token(TokenType.STAR);
			if (tokenText == "/") return new Token(TokenType.SLASH);
			if (tokenText == "%") return new Token(TokenType.PERCENT);
			if (tokenText == "^") return new Token(TokenType.CARET);
			if (tokenText == "!") return new Token(TokenType.BANG);
			if (tokenText == "(") return new Token(TokenType.LPAREN);
			if (tokenText == ")") return new Token(TokenType.RPAREN);
			if (tokenText == "=") return new Token(TokenType.ASSIGN);
		}
		// Default (shouldn't happen)
		return new Token(TokenType.UNKNOWN, tokenText);
	}

	public static Boolean IsNumber(TokenType type) {
		return type == TokenType.NUMBER;
	}

	public static Boolean IsIdentifier(TokenType type) {
		return type == TokenType.IDENTIFIER;
	}

	public static List<Token> Lex(String exprStr) {
		State state = State.START;
		List<Token> result = new List<Token>();
		String token = "";

		for (Int32 i = 0; i < exprStr.Length; i++) {
			Char c = exprStr[i];

			if (c <= ' ') {
				if (token.Length > 0) result.Add(MakeToken(token, state));
				state = State.START;
				token = "";
			} else if ((c >= '0' && c <= '9') || c == '.') {
				if (state == State.NUMBER) {
					token += c;
				} else {
					if (token.Length > 0) result.Add(MakeToken(token, state));
					state = State.NUMBER;
					token = c.ToString();  // CPP: token = String(c);
				}
			} else if (operatorChars.IndexOf(c) >= 0) {
				if (token.Length > 0) result.Add(MakeToken(token, state));
				state = State.OPERATOR;
				token = c.ToString();  // CPP: token = String(c);
				result.Add(MakeToken(token, state));
				state = State.START;
				token = "";
			} else {
				if (state == State.IDENTIFIER) {
					token += c;
				} else {
					if (token.Length > 0) result.Add(MakeToken(token, state));
					state = State.IDENTIFIER;
					token = c.ToString();  // CPP: token = String(c);
				}
			}
		}

		if (token.Length > 0) result.Add(MakeToken(token, state));
		return result;
	}
}

}

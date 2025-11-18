using System;
using System.Collections.Generic;

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

	public static Boolean IsNumber(String s) {
		if (s == null || s.Length == 0) return false;
		Char c = s[0];
		return c >= '0' && c <= '9';
	}

	public static Boolean IsIdentifier(String s) {
		if (s == null || s.Length == 0) return false;
		Char c = s[0];
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
	}

	public static List<String> Lex(String exprStr) {
		State state = State.START;
		List<String> result = new List<String>();
		String token = "";

		for (Int32 i = 0; i < exprStr.Length; i++) {
			Char c = exprStr[i];

			if (c <= ' ') {
				if (token.Length > 0) result.Add(token);
				state = State.START;
				token = "";
			} else if ((c >= '0' && c <= '9') || c == '.') {
				if (state == State.NUMBER) {
					token += c;
				} else {
					if (token.Length > 0) result.Add(token);
					state = State.NUMBER;
					token = c.ToString();  // CPP: token = String(c);
				}
			} else if (operatorChars.IndexOf(c) >= 0) {
				if (token.Length > 0) result.Add(token);
				state = State.OPERATOR;
				token = c.ToString();  // CPP: token = String(c);
			} else {
				if (state == State.IDENTIFIER) {
					token += c;
				} else {
					if (token.Length > 0) result.Add(token);
					state = State.IDENTIFIER;
					token = c.ToString();  // CPP: token = String(c);
				}
			}
		}

		if (token.Length > 0) result.Add(token);
		return result;
	}
}

}

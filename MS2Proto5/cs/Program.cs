using System;
using System.Collections.Generic;
// CPP: #include "IOHelper.g.h"
// CPP: #include "Parser.g.h"
// CPP: #include "ast.g.h"
// CPP: #include "CS_String.h"

using ASTNodeSPtr = MS2Proto5.ASTNode;

namespace MS2Proto5 {

public class Program {

	public static void Main(String[] /*args*/, Int32 /*argc*/) {
		IOHelper.Print("MS2Proto5 AST Parser with Constant Folding");
		IOHelper.Print("Enter expression to parse, or 'quit' to quit.");

		Parser parser = new Parser();

		while (true) {
			String input = IOHelper.Input("parse> ");
			if (input == null || input == "quit" || input == "exit") break;

			List<Token> tokens = Lexer.Lex(input);
			ASTNodeSPtr ast = parser.Parse(tokens);
			IOHelper.Print("Original: " + ast.ToString());

			ASTNodeSPtr simplified = ast.Simplify();
			IOHelper.Print("Simplified: " + simplified.ToString());
		}
	}
}

}

/*** BEGIN CPP_ONLY ***

int main(int argc, const char* argv[]) {
	String* args = new String[argc];
	for (int i=0; i<argc; i++) args[i] = String(argv[i]);
	MS2Proto5::Program::Main(args, argc);
}

*** END CPP_ONLY ***/

using System;
using System.Collections.Generic;
// CPP: #include "IOHelper.g.h"
// CPP: #include "Parser.g.h"
// CPP: #include "ast.g.h"

// Type alias for C++: In C#, ASTNodeUPtr is just ASTNode.
// The transpiler will generate: using ASTNodeUPtr = std::unique_ptr<ASTNode>;
using ASTNodeUPtr = MS2Proto5.ASTNode;

namespace MS2Proto5 {

public class Program {

	public static void Main(string[] args) {
		IOHelper.Print("MS2Proto5 AST Parser with Constant Folding");
		IOHelper.Print("Enter expression to parse, or 'quit' to quit.");

		Parser parser = new Parser();

		while (true) {
			String input = IOHelper.Input("parse> ");
			if (input == null || input == "quit" || input == "exit") break;

			List<Token> tokens = Lexer.Lex(input);
			ASTNodeUPtr ast = parser.Parse(tokens);
			IOHelper.Print("Original: " + ast.ToString());

			ASTNodeUPtr simplified = ast.Simplify();
			IOHelper.Print("Simplified: " + simplified.ToString());
		}
	}
}

}

/*** BEGIN CPP_ONLY ***
int main(int argc, char **argv) {
	std::vector<MS2Proto5::String> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(MS2Proto5::String(argv[i]));
	}
	MS2Proto5::Program::Main(args);
	return 0;
}
*** END CPP_ONLY ***/

using System;
using System.Collections.Generic;
// CPP: #include "IOHelper.g.h"
// CPP: #include "EvalOpSet.g.h"
// CPP: #include "Parser.g.h"
// CPP: #include "CS_Math.h"

namespace MS2Proto4 {

public class Program {

	private static Int32 errorCount = 0;
	
	private static void check(ParserPtr parser, String expression, Double expected) {
		List<Token> tokens = Lexer.Lex(expression);
		Double actual = parser.Parse(tokens);
		if (Math.Abs(actual - expected) > 0.0001) {
			IOHelper.Print(String("Unit test failure on \"") + expression + "\": expected " + ToString(expected) + ", got " + ToString(actual));
			errorCount++;
		}
	}

	public static void RunUnitTests() {
		IOHelper.Print("Unit testing: eval");

		EvalOpSet ops = new EvalOpSet();
		ParserPtr parser = new Parser(ops);
		errorCount = 0;

		check(parser, "2+2", 4);
		check(parser, "pi", 3.1415926536);
		check(parser, "2+3*4", 14);
		check(parser, "(2+3)*4", 20);
		check(parser, "5!", 120);
		check(parser, "200 - 5!", 80);
		check(parser, "2^2^3", 256);
		check(parser, "(2^2)^3", 64);
		check(parser, "round(cos(45*pi/180)*100)", 71);

		if (errorCount == 0) {
			IOHelper.Print("All tests passed.");
		} else {
			String plural = errorCount != 1 ? "s" : "";
			IOHelper.Print(ToString(errorCount) + " error" + plural + " found.");
		}
		
		// CPP: delete parser;
	}

	public static void Main(String[] /*args*/, Int32 /*argc*/) {
		RunUnitTests();

		IOHelper.Print("Enter expression to evaluate, or 'quit' to quit.");
		EvalOpSet ops = new EvalOpSet();
		Parser parser(ops);

		while (true) {
			String input = IOHelper.Input("eval> ");
			if (input == null || input == "quit" || input == "exit") break;

			List<Token> tokens = Lexer.Lex(input);
			Double result = parser.Parse(tokens);
			IOHelper.Print(result.ToString()); // CPP: printf("%f\n", result);
		}
	}
}

}

/*** BEGIN CPP_ONLY ***

int main(int argc, const char* argv[]) {
	String* args = new String[argc];
	for (int i=0; i<argc; i++) args[i] = String(argv[i]);
	MS2Proto4::Program::Main(args, argc);
}

*** END CPP_ONLY ***/


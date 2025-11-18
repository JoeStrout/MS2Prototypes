using System;
using System.Collections.Generic;

namespace MS2Proto4 {

public class Program {

	public static void RunUnitTests() {
		Console.WriteLine("Unit testing: eval");

		EvalOpSet ops = new EvalOpSet();
		Parser parser = new Parser(ops);
		Int32 errorCount = 0;

		Action<String, Double> check = (expression, expected) => {
			List<String> tokens = Lexer.Lex(expression);
			Double actual = parser.Parse(tokens);
			if (Math.Abs(actual - expected) > 0.0001) {
				Console.WriteLine("Unit test failure on \"" + expression + "\": expected " + expected + ", got " + actual);
				errorCount++;
			}
		};

		check("2+2", 4);
		check("pi", Math.PI);
		check("2+3*4", 14);
		check("(2+3)*4", 20);
		check("5!", 120);
		check("200 - 5!", 80);
		check("2^2^3", 256);
		check("(2^2)^3", 64);
		check("round(cos(45*pi/180)*100)", 71);

		if (errorCount == 0) {
			Console.WriteLine("All tests passed.");
		} else {
			String plural = errorCount != 1 ? "s" : "";
			Console.WriteLine(errorCount + " error" + plural + " found.");
		}
	}

	public static void Main(String[] args) {
		RunUnitTests();

		Console.WriteLine("Enter expression to evaluate, or 'quit' to quit.");
		EvalOpSet ops = new EvalOpSet();
		Parser parser = new Parser(ops);

		while (true) {
			Console.Write("eval> ");
			String input = Console.ReadLine();
			if (input == null || input == "quit" || input == "exit") break;

			List<String> tokens = Lexer.Lex(input);
			Double result = parser.Parse(tokens);
			Console.WriteLine(result);
		}
	}
}

}

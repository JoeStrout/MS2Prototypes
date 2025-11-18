// EvalOpSet: a parser operation set that immediately evaluates expressions
// as they are parsed, returning a numeric (double) result.
//
// Memory: this is an OpSet subclass, with one extra (small) reference
// (a Dictionary, whose storage is in MemPool managed memory).  The additional
// reference is inconsequential, but as an OpSet, this is still too big to
// copy, and so should be owned, deleted, and passed by reference or pointer.
//
// In typical usage, a method might allocate one of these on the stack, use it
// to configure a Parser, and then allow it to go out of scope -- this is fine.

using System;
using System.Collections.Generic;
// CPP: #include "OpSet.g.h"
// CPP: #include "CS_Math.h"
// CPP: #include <cmath>
// CPP: #include <iostream>

namespace MS2Proto4 {

using StringDoubleDict = Dictionary<String, Double>;
// CPP: using StringDoubleDict = Dictionary<String, Double>;

// EvalOpSet: an OpSet implementation that immediately evaluates expressions.
// This maintains a dictionary of variables and performs arithmetic operations
// on the fly, returning numerical results.
public class EvalOpSet : OpSet {
	private StringDoubleDict _vars;

	public EvalOpSet() {
		_vars = new StringDoubleDict(); // CPP:

		// Initialize with standard mathematical constants and functions
		_vars["pi"] = 3.1415926536; // Math.PI;
		_vars["e"] = 2.7182818285; // Math.E;

		// Define operation delegates
		numVal = (String s) => {
			Double result = 0.0; // CPP: Double result = atof(s.c_str());
			Double.TryParse(s, out result); // CPP:
			return result;
		};

		getVar = (String id) => {  // needs [this] in its C++ lambda form
			Double result = 0.0;
			if (!_vars.TryGetValue(id, out result)) {
				Console.WriteLine("Undefined identifier: " + id); // CPP: std::cout << "Undefined identifier: " << id.c_str() << std::endl;
			}
			return result;
		};

		setVar = (String id, Double val) => {  // also needs [this]
			_vars[id] = val;
			return val;
		};

		call = (String id, Double arg) => {
			// Built-in functions
//			if (id == "sqrt") return Math.Sqrt(arg);
//			if (id == "sin") return Math.Sin(arg);
			if (id == "cos") return Math.Cos(arg);
//			if (id == "tan") return Math.Tan(arg);
//			if (id == "asin") return Math.Asin(arg);
//			if (id == "acos") return Math.Acos(arg);
//			if (id == "atan") return Math.Atan(arg);
//			if (id == "log") return Math.Log(arg);
			if (id == "floor") return Math.Floor(arg);
			if (id == "ceil") return Math.Ceiling(arg);
			if (id == "round") return Math.Round(arg);
//			if (id == "sign") return Math.Sign(arg);

			Console.WriteLine("Undefined function: " + id); // CPP: std::cout << "Undefined function: " << id.c_str() << std::endl;
			return 0.0;
		};

		add = (Double a, Double b) => a + b;
		subtract = (Double a, Double b) => a - b;
		multiply = (Double a, Double b) => a * b;
		divide = (Double a, Double b) => a / b;
		power = (Double a, Double b) => Math.Pow(a, b);
		unaryMinus = (Double x) => -x;

		mod = (Double a, Double b) => {
			return a % b; // CPP: return fmod(a, b);
		};

		factorial = (Double x) => {
			Double result = x;
			while (x > 1) {
				x -= 1;
				result *= x;
			}
			return result;
		};
	}
}

}

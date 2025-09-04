using System;
using System.Collections.Generic;
// CPP: #include "Bytecode.g.h"
// CPP: #include "StringUtils.g.h"

namespace MiniScript {

	public class Assembler {

		public FuncDef Current; // function we are currently building

		public static List<String> GetTokens(String line) {
			// Clean the string, stripping off comment at '#',
			// and divide into tokens by whitespace and commas.  Example:
			//  "   LOAD r5, r6 # comment"  -->  ["LOAD", "r5", "r6"]
			
			// Parse line character by character, collecting tokens
			List<String> tokens = new List<String>();
			Int32 tokenStart = -1;
			
			for (Int32 i = 0; i < line.Length; i++) {
				Char c = line[i];
				
				// Stop at comment
				if (c == '#') break;
				
				// Check if this is a delimiter (whitespace or comma)
				if (c == ' ' || c == '\t' || c == ',') {
					// End current token if we have one
					if (tokenStart >= 0) {
						tokens.Add(line.Substring(tokenStart, i - tokenStart));
						tokenStart = -1;
					}
				} else {
					// Start new token if not already started
					if (tokenStart < 0) {
						tokenStart = i;
					}
				}
			}
			
			// Add final token if any
			if (tokenStart >= 0) {
				tokens.Add(line.Substring(tokenStart, line.Length - tokenStart));
			}
			return tokens;
		}
		
		public UInt32 AddLine(String line) {
			List<String> parts = GetTokens(line);
			// ToDo
			return 0;
		}

	}
}

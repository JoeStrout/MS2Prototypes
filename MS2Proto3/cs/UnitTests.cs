// This module gathers together all the unit tests for this prototype.
// Each test returns true on success, false on failure.

using System;
using System.Collections.Generic;
// CPP: #include "IOHelper.g.h"
// CPP: #include "StringUtils.g.h"
// CPP: #include "Disassembler.g.h"
// CPP: #include "Assembler.g.h"  // We really should automate this.

namespace MiniScript {

	public static class UnitTests {
	
		public static Boolean Assert(Boolean condition, String message) {
			if (condition) return true;
			IOHelper.Print(new String("Unit test failure: ") + message);
			return false;
		}
		
		public static Boolean AssertEqual(String actual, String expected) {
			if (actual == expected) return true;
			Assert(false, new String("Unit test failure: expected \"")
			  + expected + "\" but got \"" + actual + "\"");
			return false;
		}
			
		public static Boolean AssertEqual(List<String> actual, List<String> expected) {
			Boolean ok = true;
			if ((actual == null) != (expected == null)) ok = false; // CPP: // (no nulls)
			if (ok && actual.Count != expected.Count) ok = false;
			for (Int32 i = 0; ok && i < actual.Count; i++) {
				if (actual[i] != expected[i]) ok = false;
			}
			if (ok) return true;
			Assert(false, new String("Unit test failure: expected ")
			  + StringUtils.Str(expected) + " but got " + StringUtils.Str(actual));
			return false;
		}
			
		public static Boolean TestStringUtils() {
			return 
				AssertEqual(StringUtils.ToHex((UInt32)123456789), "075BCD15")
			&&  AssertEqual(new String("abcdef").Left(3), "abc")
			&&	AssertEqual(new String("abcdef").Right(3), "def");
		}
		
		public static Boolean TestDisassembler() {
			return
				AssertEqual(Disassembler.ToString(0x01050A00), "LOAD   r5, r10");
		}
		
		public static Boolean TestAssembler() {
			return
				AssertEqual(Assembler.GetTokens("   LOAD r5, r6 # comment"),
				  new List<String> { "LOAD", "r5", "r6" })
			&&  AssertEqual(Assembler.GetTokens("  NOOP  "),
				  new List<String> { "NOOP" })
			&&  AssertEqual(Assembler.GetTokens(" # comment only"),
				  new List<String>());
				  
		}
		
		public static Boolean RunAll() {
			return TestStringUtils()
				&& TestDisassembler()
				&& TestAssembler();
		}
	}

}

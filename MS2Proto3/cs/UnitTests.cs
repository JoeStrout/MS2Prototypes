// This module gathers together all the unit tests for this prototype.
// Each test returns true on success, false on failure.

using System;
// CPP: #include "IOHelper.g.h"
// CPP: #include "StringUtils.g.h"
// CPP: #include "Disassembler.g.h"

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
		
		public static Boolean RunAll() {
			return TestStringUtils()
				&& TestDisassembler();
		}
	}

}

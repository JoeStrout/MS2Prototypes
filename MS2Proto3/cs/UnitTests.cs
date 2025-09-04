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
			
		public static Boolean AssertEqual(UInt32 actual, UInt32 expected) {
			if (actual == expected) return true;
			Assert(false, new String("Unit test failure: expected 0x")
			  + StringUtils.ToHex(expected) + "\" but got 0x" + StringUtils.ToHex(actual));
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
			// Test tokenization
			Boolean tokensOk = 
				AssertEqual(Assembler.GetTokens("   LOAD r5, r6 # comment"),
				  new List<String> { "LOAD", "r5", "r6" })
			&&  AssertEqual(Assembler.GetTokens("  NOOP  "),
				  new List<String> { "NOOP" })
			&&  AssertEqual(Assembler.GetTokens(" # comment only"),
				  new List<String>());
			
			if (!tokensOk) return false;
			
			// Test instruction assembly
			Assembler asm = new Assembler();
			
			// Test NOOP
			Boolean asmOk = AssertEqual(asm.AddLine("NOOP"), 
				BytecodeUtil.INS(Opcode.NOOP));
			
			// Test LOAD variants
			asmOk = asmOk && AssertEqual(asm.AddLine("LOAD r5, r3"), 
				BytecodeUtil.INS_ABC(Opcode.LOAD_rA_rB, 5, 3, 0));
			
			asmOk = asmOk && AssertEqual(asm.AddLine("LOAD r2, 42"), 
				BytecodeUtil.INS_AB(Opcode.LOAD_rA_iBC, 2, 42));
			
			asmOk = asmOk && AssertEqual(asm.AddLine("LOAD r7, k15"), 
				BytecodeUtil.INS_AB(Opcode.LOAD_rA_kBC, 7, 15));
			
			// Test arithmetic
			asmOk = asmOk && AssertEqual(asm.AddLine("ADD r1, r2, r3"), 
				BytecodeUtil.INS_ABC(Opcode.ADD_rA_rB_rC, 1, 2, 3));
			
			asmOk = asmOk && AssertEqual(asm.AddLine("SUB r4, r5, r6"), 
				BytecodeUtil.INS_ABC(Opcode.SUB_rA_rB_rC, 4, 5, 6));
			
			// Test control flow
			asmOk = asmOk && AssertEqual(asm.AddLine("JUMP 10"), 
				BytecodeUtil.INS(Opcode.JUMP_iABC) | (UInt32)(10 & 0xFFFFFF));
			
			asmOk = asmOk && AssertEqual(asm.AddLine("IFLT r8, r9"), 
				BytecodeUtil.INS_ABC(Opcode.IFLT_rA_rB, 8, 9, 0));
			
			asmOk = asmOk && AssertEqual(asm.AddLine("RETURN"), 
				BytecodeUtil.INS(Opcode.RETURN));
			
			return asmOk;
		}
		
		public static Boolean RunAll() {
			return TestStringUtils()
				&& TestDisassembler()
				&& TestAssembler();
		}
	}

}

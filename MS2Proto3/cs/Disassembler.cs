using System;
using System.Collections.Generic;
// CPP: #include "Bytecode.g.h"
// CPP: #include "StringUtils.g.h"

namespace MiniScript {

	public static class Disassembler {
	
		public static String ToString(UInt32 instruction) {
			Opcode opcode = (Opcode)BytecodeUtil.OP(instruction);
			String mnemonic = BytecodeUtil.ToMnemonic(opcode);
			mnemonic = (mnemonic + "    ").Left(6);
			String a = StringUtils.ToHex(BytecodeUtil.A(instruction));
			
			return mnemonic + a;
		}
	
	}

}

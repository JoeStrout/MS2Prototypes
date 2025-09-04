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
			
			switch (opcode) {
				case Opcode.NOOP:
				case Opcode.RETURN:
					return mnemonic;
				case Opcode.MOVE:
					return StringUtils.Format("{0} r{1}, r{2}",
						mnemonic,
						BytecodeUtil.A(instruction),
						BytecodeUtil.B(instruction));
        		case Opcode.LOADK:
        			return StringUtils.Format("{0} r{1}, k{2}",
        				mnemonic,
        				BytecodeUtil.A(instruction),
        				BytecodeUtil.BCu(instruction));
        		case Opcode.LOADI:
        			return StringUtils.Format("{0} r{1}, {2}",
        				mnemonic,
        				BytecodeUtil.A(instruction),
        				BytecodeUtil.BC(instruction));
        		case Opcode.ADD:
        		case Opcode.SUB:
        			return StringUtils.Format("{0} r{1}, r{2}, r{3}",
        				mnemonic,
        				BytecodeUtil.A(instruction),
        				BytecodeUtil.B(instruction),
        				BytecodeUtil.C(instruction));
        		case Opcode.JMP:
        			return StringUtils.Format("{0} {1}",
        				mnemonic,
        				BytecodeUtil.BC(instruction));	// ToDo: 24-bit jump instead?
        		case Opcode.IFLT:
        			return StringUtils.Format("{0} r{1}, r{2}, {3}",
        				mnemonic,
        				BytecodeUtil.A(instruction),
        				BytecodeUtil.B(instruction),
        				(SByte)BytecodeUtil.C(instruction));
				case Opcode.CALLF:
        			return StringUtils.Format("{0} {1}, {2}, {3}",
        				mnemonic,
        				BytecodeUtil.A(instruction),
        				BytecodeUtil.B(instruction),
        				BytecodeUtil.C(instruction));
				default:
					return new String("??? ") + StringUtils.ToHex(instruction);
			}
		}
	
	}

}

using System;
using System.Collections.Generic;
using System.Linq;	// only for ToList!
using MiniScript;
// CPP: #include "UnitTests.g.h"
// CPP: using namespace MiniScript;

public class Program {
	public static void Main(string[] args) {
		//*** BEGIN CS_ONLY ***
		// The args passed to the C# main program do not include the program path.
		// To get an arg list like what C++ gets, we must do:
		args = Environment.GetCommandLineArgs();
		Int32 argCount = args.Length;
		//*** END CS_ONLY ***
		
		IOHelper.Print("MiniScript 2.0 Prototype 3");
		IOHelper.Print(
			"Build: C# version" // CPP: "Build: C++ version"
		);
		IOHelper.Print("Milestone 1: complete!");
		IOHelper.Print("Milestone 2: in progress");
		
		IOHelper.Print("Running unit tests...");
		if (!UnitTests.RunAll()) return;
		IOHelper.Print("Unit tests complete.");
		
		IOHelper.Print(StringUtils.Format("Got {0} args", argCount));
		for (Int32 i=0; i<argCount; i++) {
			IOHelper.Print(StringUtils.Format("{0}: {1}", i, args[i]));
		}
		
		// Check for assembly file argument
		if (argCount > 1) {
			String filePath = args[1];
			IOHelper.Print(StringUtils.Format("Reading assembly file: {0}", filePath));
			
			List<String> lines = IOHelper.ReadFile(filePath);
			if (lines.Count == 0) {
				IOHelper.Print("No lines read from file.");
				return;
			}
			
			IOHelper.Print(StringUtils.Format("Assembling {0} lines...", lines.Count));
			Assembler assembler = new Assembler();
			
			// Use two-pass assembly for label support
			assembler.Assemble(lines);
			IOHelper.Print("Assembly complete.");
			
			// Disassemble and print the code
			IOHelper.Print("Disassembly:");
			List<String> disassembly = Disassembler.Disassemble(assembler.Current, true);
			for (Int32 i = 0; i < disassembly.Count; i++) {
				IOHelper.Print(disassembly[i]);
			}
		}
		
		IOHelper.Print("All done!");
	}
}

/*** BEGIN CPP_ONLY ***

int main(int argc, const char* argv[]) {
	String* args = new String[argc];
	for (int i=0; i<argc; i++) args[i] = String(argv[i]);
	Program::Main(args, argc);
}

*** END CPP_ONLY ***/

using System;
using System.Collections.Generic;
using System.Linq;	// only for ToList!
using MiniScript;
// CPP: #include "UnitTests.g.h"
// CPP: #include "VM.g.h"
// CPP: #include "gc.h"
// CPP: using namespace MiniScript;

public class Program {
	public static void Main(string[] args) {
		// CPP: gc_init();
	
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
		IOHelper.Print("Milestone 3: complete!");
		IOHelper.Print("Milestone 4: about to begin");
		
		IOHelper.Print("Running unit tests...");
		if (!UnitTests.RunAll()) return;
		IOHelper.Print("Unit tests complete.");
		
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
			
			// Use multi-function assembly with @function: label support
			assembler.Assemble(lines);
			IOHelper.Print("Assembly complete.");
			
			// Disassemble and print program
			IOHelper.Print("Disassembly:\n");
			List<String> disassembly = Disassembler.Disassemble(assembler.Functions, true);
			for (Int32 i = 0; i < disassembly.Count; i++) {
				IOHelper.Print(disassembly[i]);
			}
			
			// Print all functions found
			IOHelper.Print(StringUtils.Format("Found {0} functions:", assembler.Functions.Count));
			for (Int32 i = 0; i < assembler.Functions.Count; i++) {
				FuncDef func = assembler.Functions[i];
				IOHelper.Print(StringUtils.Format("  {0}: {1} instructions, {2} constants", 
					func.Name, func.Code.Count, func.Constants.Count));
			}
			
			// Execute the program with the VM
			IOHelper.Print("");
			IOHelper.Print("Executing @main with VM...");
			
			VM vm = new VM();
			Value result = vm.Run(assembler.Functions);
			
			IOHelper.Print(StringUtils.Format("VM execution complete. Result in r0: {0}", result));
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

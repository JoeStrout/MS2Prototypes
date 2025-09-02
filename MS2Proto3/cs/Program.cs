using System;
using System.Collections.Generic;
using System.Linq;	// only for ToList!

public class Program {
	public static void Main(String[] args) {
		// CPP: (void)args;
		IOHelper.Print("MiniScript 2.0 Prototype 3");
		IOHelper.Print(
			"Build: C# version" // CPP: "Build: C++ version"
		);
		IOHelper.Print("Milestone 1: complete!");
		
		String text = IOHelper.Input("Enter some words: ");
		
		IOHelper.Print("Splitting...");
        List<String> words = text.Split(' ').ToList();

        IOHelper.Print("Reversing...");
        words.Reverse();
        
        IOHelper.Print("Joining...");
        String reversed = String.Join(" ", words);
        
        IOHelper.Print(reversed);
        IOHelper.Print("All done!");
	}
}


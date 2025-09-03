// IOHelper
//	This is a simple wrapper for console output on each platform.

using System;
// CPP: #include <iostream>
// CPP: #include <stdio.h>

public static class IOHelper {

	public static void Print(String message) {
		Console.WriteLine(message);  // CPP: std::cout << message.c_str() << std::endl;
	}
	
	public static String Input(String prompt) {
		//*** BEGIN CS_ONLY ***
		Console.Write(prompt);
		return Console.ReadLine() ?? "";
		//*** END CS_ONLY ***
		/*** BEGIN CPP_ONLY
		std::cout << prompt.c_str();
		char *line = NULL;
		size_t len = 0;
		
		String result;
		int bytes = getline(&line, &len, stdin);
		if (bytes != -1) {
			line[strcspn (line, "\n")] = 0;   // trim \n
			result = line;
			free(line);
		}
		return result;
		*** END CPP_ONLY ***/
	}
	
}

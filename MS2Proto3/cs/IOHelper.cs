// IOHelper
//	This is a simple wrapper for console output on each platform.

using System;
// CPP: #include <iostream>

public static class IOHelper {

    public static void Print(String message) {
        Console.WriteLine(message);  // CPP: std::cout << message.c_str() << std::endl;
    }

}

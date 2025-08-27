// IOHelper
//	This is a simple wrapper for console output on each platform.

using System;
#include <iostream>


    static void IOHelper::print(string message) {
        std::cout << message.c_str() << std::endl;
    }


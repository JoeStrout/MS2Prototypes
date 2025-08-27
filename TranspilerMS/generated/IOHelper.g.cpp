// IOHelper
//	This is a simple wrapper for console output on each platform.

#include "IOHelper.g.h"
#include <iostream>


    void IOHelper::print(string message) {
        std::cout << message.c_str() << std::endl;
    }


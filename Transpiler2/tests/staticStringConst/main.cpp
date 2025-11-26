#include <iostream>
#include "expected/Op.g.h"

using namespace MiniScript;

int main() {
	std::cout << "Op.PLUS: " << Op::PLUS.c_str() << std::endl;
	std::cout << "Op.MINUS: " << Op::MINUS.c_str() << std::endl;
	std::cout << "Op.TIMES: " << Op::TIMES.c_str() << std::endl;
	return 0;
}

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -O2

# Object files
OBJS = string.o StringStorage.o StringPool.o unicodeUtil.o

# Test programs
TEST_PROGS = test_string

all: $(TEST_PROGS)

test_string: test_string.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

string.o: string.cpp string.h StringStorage.h StringPool.h
	$(CXX) $(CXXFLAGS) -c string.cpp

StringStorage.o: StringStorage.cpp StringStorage.h unicodeUtil.h
	$(CXX) $(CXXFLAGS) -c StringStorage.cpp

StringPool.o: StringPool.cpp StringPool.h StringStorage.h unicodeUtil.h
	$(CXX) $(CXXFLAGS) -c StringPool.cpp

unicodeUtil.o: unicodeUtil.c unicodeUtil.h
	gcc -c unicodeUtil.c

test: test_string
	./test_string

clean:
	rm -f $(OBJS) $(TEST_PROGS)
	rm -rf *.dSYM

.PHONY: all test clean
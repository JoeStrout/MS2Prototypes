CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra -I..
TARGET = numberWords_classic
SOURCES = numberWords.cpp ../MiniscriptTypes.cpp ../SimpleString.cpp ../List.cpp ../Dictionary.cpp ../QA.cpp ../SplitJoin.cpp ../UnicodeUtil.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
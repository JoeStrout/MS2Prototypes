CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -O2

# CodeBase directory
CODEBASE_DIR = ../../CodeBase

# Include paths
INCLUDES = -I$(CODEBASE_DIR)

# CodeBase object files
CODEBASE_OBJS = $(CODEBASE_DIR)/StringStorage.o $(CODEBASE_DIR)/StringPool.o \
                $(CODEBASE_DIR)/unicodeUtil.o $(CODEBASE_DIR)/MemPool.o

# Generated source files
SOURCES = Program.g.cpp IOHelper.g.cpp
HEADERS = Program.g.h IOHelper.g.h
OBJS = Program.g.o IOHelper.g.o

# Target executable
TARGET = Program

all: $(TARGET)

$(TARGET): $(OBJS) $(CODEBASE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

Program.g.o: Program.g.cpp Program.g.h IOHelper.g.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c Program.g.cpp

IOHelper.g.o: IOHelper.g.cpp IOHelper.g.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c IOHelper.g.cpp

# Build CodeBase dependencies if needed
$(CODEBASE_OBJS):
	$(MAKE) -C $(CODEBASE_DIR)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf *.dSYM

clean-all: clean
	$(MAKE) -C $(CODEBASE_DIR) clean

test: $(TARGET)
	./$(TARGET)

.PHONY: all clean clean-all test
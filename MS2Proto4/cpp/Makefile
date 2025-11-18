# MS2Proto4 C++ Makefile
# Builds the transpiled C++ version

CXX = g++
CC = gcc

CXXFLAGS = -std=gnu++11 -Wall -Wextra -O3 -DNDEBUG -Icore
CFLAGS = -std=c99 -Wall -Wextra -O3 -DNDEBUG -Icore
SRCDIR = core
GENDIR = ../generated
BUILDDIR = ../build/cpp
OBJDIR = $(BUILDDIR)/obj

# Core C++ and C files (excluding test programs)
CORE_CPP_SOURCES = $(filter-out $(SRCDIR)/test_%.cpp $(SRCDIR)/debug_%.cpp, $(wildcard $(SRCDIR)/*.cpp))
CORE_C_SOURCES = $(wildcard $(SRCDIR)/*.c)
CORE_CPP_OBJECTS = $(CORE_CPP_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/core_%.o)
CORE_C_OBJECTS = $(CORE_C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/core_%.o)
CORE_OBJECTS = $(CORE_CPP_OBJECTS) $(CORE_C_OBJECTS)

# Generated C++ files (will be populated by transpiler)
GEN_SOURCES = $(wildcard $(GENDIR)/*.g.cpp)
GEN_OBJECTS = $(GEN_SOURCES:$(GENDIR)/%.g.cpp=$(OBJDIR)/gen_%.o)

# All objects
OBJECTS = $(CORE_OBJECTS) $(GEN_OBJECTS)

# Target executable
TARGET = $(BUILDDIR)/MS2Proto4

.PHONY: all clean info

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(OBJECTS) -o $@

# Core C++ object files
$(OBJDIR)/core_%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Core C object files
$(OBJDIR)/core_%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Generated C++ object files
$(OBJDIR)/gen_%.o: $(GENDIR)/%.g.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create directories
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(BUILDDIR)

# Show build info
info:
	@echo "Core C++ sources: $(CORE_CPP_SOURCES)"
	@echo "Core C sources: $(CORE_C_SOURCES)"
	@echo "Generated sources: $(GEN_SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Target: $(TARGET)"

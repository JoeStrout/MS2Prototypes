CC = cc
CFLAGS = -O3 -Wall -Wextra
TARGET = tinyvm
OBJS = tiny_vm.o main.o

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Object files
tiny_vm.o: tiny_vm.c tiny_vm.h
	$(CC) $(CFLAGS) -c tiny_vm.c

main.o: main.c tiny_vm.h
	$(CC) $(CFLAGS) -c main.c

# Force switch dispatch mode
switch: CFLAGS += -DVM_USE_COMPUTED_GOTO=0
switch: clean $(TARGET)

# Force computed goto mode
goto: CFLAGS += -DVM_USE_COMPUTED_GOTO=1
goto: clean $(TARGET)

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean switch goto
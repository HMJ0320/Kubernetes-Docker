CC = gcc

# Compiler flags
CFLAGS = -Wall -g -Wextra

# Source and Object files
SRC = main.c ../src/linkedlist.c
OBJ = main.o linkedlist.o

# Output executable
OUT = main

# Default target
all: $(OUT)

# Rule for compiling the source file
$(OBJ): $(SRC) ../include/linkedlist.h
	$(CC) $(CFLAGS) -c $(SRC)

# Rule for linking the object files into the final executable
$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT)

# Runs the output executable
run: $(OUT)
	./$(OUT)

test: $(OUT)
	@echo "Running tests ..."
	../tests/test

# Clean up the generated files
clean:
	rm -f $(OBJ) $(OUT)

.PHONY: all run test clean
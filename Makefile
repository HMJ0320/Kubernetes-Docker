CC = gcc

# Compiler flags
CFLAGS = -Wall -g -Wextra

# Source and Object files
SRC = src/main.c src/cli.c src/hashtable.c src/linkedlist.c src/network.c src/container.c 
OBJ = main.o cli.o hashtable.o linkedlist.o network.o container.o

# Output executable
OUT = bin/container

# Default target
all: $(OUT)

# Rule for compiling the source file
$(OBJ): $(SRC) include/cli.h include/hashtable.h include/linkedlist.h include/network.h include/container.h 
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
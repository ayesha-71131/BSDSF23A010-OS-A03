# Makefile for myshell project
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lreadline

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/shell.c $(SRC_DIR)/execute.c
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/myshell

# Ensure directories exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJECTS)
	@echo "[LD] Linking $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[CC] Compiling $< -> $@"
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# Run the shell
run: $(TARGET)
	@echo "--------------------------------------"
	@echo " Starting Custom Shell (myshell)"
	@echo "--------------------------------------"
	./$(TARGET)

# Clean up
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
	@echo "Cleanup complete."

.PHONY: all run clean

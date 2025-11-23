# Makefile for CSC3150 Assignment 2

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -pthread

# Target executable
TARGET = hw2

# Source file
SRC = hw2.cpp

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean build files
clean:
	rm -f $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run


# Define the build directory
BUILD_DIR := build

# Default target
all: build run

# Create the build directory and run cmake and make
build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make

# Run the CallGraphAnalyzer inside the build directory
run: build
	cd $(BUILD_DIR) && ./CallGraphAnalyzer ../../input/input.cpp

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)
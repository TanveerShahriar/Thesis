# Define the build directory
BUILD_DIR := build

# Default target
all: build

# Create the build directory and run cmake and make
build:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)
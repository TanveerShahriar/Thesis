# Define subdirectory
CALL_GRAPH_DIR := Obfuscator

# Default target
all:
	$(MAKE) -C $(CALL_GRAPH_DIR)

# Clean the build directory in the subdirectory
clean:
	$(MAKE) -C $(CALL_GRAPH_DIR) clean
	rm -rf output
# Define subdirectory
CALL_GRAPH_DIR := call_graph

# Default target
all:
	$(MAKE) -C $(CALL_GRAPH_DIR)

# Clean the build directory in the subdirectory
clean:
	$(MAKE) -C $(CALL_GRAPH_DIR) clean
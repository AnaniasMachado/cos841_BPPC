CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

# Folders
SRC_DIR = .
TEST_DIR = test
UTIL_DIR = util
BIN_DIR = bin

# Files
TEST_BPPC = $(TEST_DIR)/test_bppc.cpp
UTIL_SRC = $(UTIL_DIR)/bppc.cpp

# Object files (generated in same directory)
TEST_BPPC_OBJ = test_bppc.o
UTIL_OBJ = bppc.o

# Target executable
TEST_BPPC_BIN = $(BIN_DIR)/test_bppc

# Default target
all: $(TEST_BPPC_BIN)

# Link
$(TEST_BPPC_BIN): $(TEST_BPPC_OBJ) $(UTIL_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile test file
$(TEST_BPPC_OBJ): $(TEST_BPPC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile util file
$(UTIL_OBJ): $(UTIL_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f *.o $(BIN_DIR)/*
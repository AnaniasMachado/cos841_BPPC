CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

# Folders
SRC_DIR = .
TEST_DIR = test
UTIL_DIR = util
MT_DIR = metaheuristic
BIN_DIR = bin

# Ensure bin folder exists
$(shell mkdir -p $(BIN_DIR))

# -------------------- Source files --------------------
UTIL_SRCS = $(UTIL_DIR)/bppc.cpp \
            $(UTIL_DIR)/solution.cpp \
            $(UTIL_DIR)/builder.cpp \
            $(UTIL_DIR)/local_search.cpp \
            $(UTIL_DIR)/perturbations.cpp \
            $(MT_DIR)/ails.cpp

# -------------------- Test programs --------------------
TEST_BPPC = $(TEST_DIR)/test_bppc.cpp
TEST_SOLUTION = $(TEST_DIR)/test_solution.cpp
TEST_LOCAL = $(TEST_DIR)/test_local_search.cpp
TEST_PERT = $(TEST_DIR)/test_perturbations.cpp
TEST_AILS = $(TEST_DIR)/test_ails.cpp

# -------------------- Object files --------------------
TEST_BPPC_OBJ = $(BIN_DIR)/test_bppc.o
TEST_SOLUTION_OBJ = $(BIN_DIR)/test_solution.o
TEST_LOCAL_OBJ = $(BIN_DIR)/test_local_search.o
TEST_PERT_OBJ = $(BIN_DIR)/test_perturbations.o
TEST_AILS_OBJ = $(BIN_DIR)/test_ails.o

UTIL_OBJS = $(BIN_DIR)/bppc.o $(BIN_DIR)/solution.o $(BIN_DIR)/builder.o \
            $(BIN_DIR)/local_search.o $(BIN_DIR)/perturbations.o $(BIN_DIR)/ails.o

# -------------------- Target executables --------------------
TEST_BPPC_BIN = $(BIN_DIR)/test_bppc
TEST_SOLUTION_BIN = $(BIN_DIR)/test_solution
TEST_LOCAL_BIN = $(BIN_DIR)/test_local_search
TEST_PERT_BIN = $(BIN_DIR)/test_perturbations
TEST_AILS_BIN = $(BIN_DIR)/test_ails

# -------------------- Default target --------------------
all: $(TEST_BPPC_BIN) $(TEST_SOLUTION_BIN) $(TEST_LOCAL_BIN) $(TEST_PERT_BIN) $(TEST_AILS_BIN)

# -------------------- Linking --------------------
$(TEST_BPPC_BIN): $(TEST_BPPC_OBJ) $(UTIL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_SOLUTION_BIN): $(TEST_SOLUTION_OBJ) $(UTIL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_LOCAL_BIN): $(TEST_LOCAL_OBJ) $(UTIL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_PERT_BIN): $(TEST_PERT_OBJ) $(UTIL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_AILS_BIN): $(TEST_AILS_OBJ) $(UTIL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# -------------------- Compilation --------------------
# Test objects
$(BIN_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Utility objects
$(BIN_DIR)/bppc.o: $(UTIL_DIR)/bppc.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/solution.o: $(UTIL_DIR)/solution.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/builder.o: $(UTIL_DIR)/builder.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/local_search.o: $(UTIL_DIR)/local_search.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/perturbations.o: $(UTIL_DIR)/perturbations.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/ails.o: $(MT_DIR)/ails.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -------------------- Clean --------------------
clean:
	rm -f $(BIN_DIR)/*.o $(BIN_DIR)/*
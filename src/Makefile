# -------------------- Compiler --------------------
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

# -------------------- HiGHS --------------------
HIGHS_INC = -I/usr/include/highs
HIGHS_LIB = -lhighs

# -------------------- Directories --------------------
UTIL_DIR = util
MT_DIR = metaheuristic
TEST_DIR = test
BIN_DIR = bin

# -------------------- Ensure bin exists --------------------
$(shell mkdir -p $(BIN_DIR))

# -------------------- Source files --------------------
UTIL_SRCS = $(UTIL_DIR)/bppc.cpp \
            $(UTIL_DIR)/solution.cpp \
            $(UTIL_DIR)/builder.cpp \
            $(UTIL_DIR)/local_search.cpp \
            $(UTIL_DIR)/perturbations.cpp \
            $(UTIL_DIR)/rvnd.cpp \
            $(UTIL_DIR)/qrvnd.cpp \
            $(UTIL_DIR)/bks.cpp

MT_SRCS = $(MT_DIR)/ails.cpp

TEST_SRCS = $(TEST_DIR)/test_bppc.cpp \
            $(TEST_DIR)/test_solution.cpp \
            $(TEST_DIR)/test_local_search.cpp \
            $(TEST_DIR)/test_perturbations.cpp \
            $(TEST_DIR)/test_ails.cpp \
            $(TEST_DIR)/test_builder.cpp \
            $(TEST_DIR)/test_assignment.cpp

# -------------------- Object files --------------------
UTIL_OBJS = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(notdir $(UTIL_SRCS)))
MT_OBJS   = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(notdir $(MT_SRCS)))
TEST_OBJS = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(notdir $(TEST_SRCS)))

# -------------------- Executables --------------------
TEST_BINS = $(patsubst %.cpp,$(BIN_DIR)/%,$(notdir $(TEST_SRCS)))
AUX_BIN = $(BIN_DIR)/ails_runner

# -------------------- Default target --------------------
all: $(TEST_BINS) $(AUX_BIN)

# -------------------- Compile utility objects --------------------
$(BIN_DIR)/%.o: $(UTIL_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(HIGHS_INC) -c $< -o $@

# -------------------- Compile metaheuristic objects --------------------
$(BIN_DIR)/%.o: $(MT_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(HIGHS_INC) -c $< -o $@

# -------------------- Compile test objects --------------------
$(BIN_DIR)/%.o: $(TEST_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(HIGHS_INC) -c $< -o $@

# -------------------- Link test executables --------------------
$(BIN_DIR)/%: $(BIN_DIR)/%.o $(UTIL_OBJS) $(MT_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(HIGHS_LIB)

# -------------------- Link AILS runner --------------------
$(AUX_BIN): $(UTIL_DIR)/ails_runner.cpp $(UTIL_OBJS) $(MT_OBJS)
	$(CXX) $(CXXFLAGS) $(HIGHS_INC) $^ -o $@ $(HIGHS_LIB)

# -------------------- Ensure bin exists --------------------
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# -------------------- Clean --------------------
clean:
	rm -rf $(BIN_DIR)/*
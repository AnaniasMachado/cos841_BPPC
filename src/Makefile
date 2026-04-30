# -------------------- Compiler --------------------
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -I$(UTIL_DIR) -I$(MT_DIR) -I$(TEST_DIR)

# -------------------- HiGHS --------------------
HIGHS_INC = -I/usr/include/highs
HIGHS_LIB = -lhighs

# -------------------- Gurobi --------------------
# Adjust if your installation path differs
GUROBI_HOME = /home/pc-515/gurobi1301/linux64
GUROBI_INC = -I$(GUROBI_HOME)/include
GUROBI_LIB = -L$(GUROBI_HOME)/lib -lgurobi_c++ -lgurobi130

# Runtime library path
export LD_LIBRARY_PATH := $(GUROBI_HOME)/lib:$(LD_LIBRARY_PATH)

# -------------------- Directories --------------------
UTIL_DIR = util
MT_DIR = metaheuristic
TEST_DIR = test
EXP_DIR = experiment
IRACE_DIR = irace
BIN_DIR = bin

# -------------------- Ensure bin exists --------------------
$(shell mkdir -p $(BIN_DIR))

# -------------------- Source files --------------------
UTIL_SRCS = bppc.cpp \
            solution.cpp \
            builder.cpp \
            local_search.cpp \
            perturbations.cpp \
            rvnd.cpp \
            qrvnd.cpp \
            bks.cpp \
            experiment_runner.cpp

MT_SRCS = ails.cpp \
          ils.cpp

TEST_SRCS = test_bppc.cpp \
            test_solution.cpp \
            test_local_search.cpp \
            test_perturbations.cpp \
            test_ails.cpp \
            test_ils.cpp \
            test_builder.cpp \
            test_assignment.cpp \
            test_kempe.cpp

MAIN_SRCS = run_ails.cpp \
            ails_runner.cpp

# -------------------- VPATH --------------------
VPATH = $(UTIL_DIR) $(MT_DIR) $(TEST_DIR) $(EXP_DIR) $(IRACE_DIR)

# -------------------- Object files --------------------
UTIL_OBJS = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(UTIL_SRCS))
MT_OBJS   = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(MT_SRCS))
TEST_OBJS = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(TEST_SRCS))
MAIN_OBJS = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(MAIN_SRCS))

# -------------------- Executables --------------------
TEST_BINS = $(patsubst %.cpp,$(BIN_DIR)/%,$(TEST_SRCS))
MAIN_BINS = $(patsubst %.cpp,$(BIN_DIR)/%,$(MAIN_SRCS))

# -------------------- Default target --------------------
all: $(TEST_BINS) $(MAIN_BINS)

# -------------------- Compile objects --------------------
$(BIN_DIR)/%.o: %.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(HIGHS_INC) $(GUROBI_INC) -c $< -o $@

# -------------------- Link test executables --------------------
$(BIN_DIR)/test_%: $(BIN_DIR)/test_%.o $(UTIL_OBJS) $(MT_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(HIGHS_LIB) $(GUROBI_LIB) \
	    -Wl,-rpath,$(GUROBI_HOME)/lib

# -------------------- Link main programs --------------------
$(BIN_DIR)/run_ails: $(BIN_DIR)/run_ails.o $(UTIL_OBJS) $(MT_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(HIGHS_LIB) $(GUROBI_LIB) \
	    -Wl,-rpath,$(GUROBI_HOME)/lib

$(BIN_DIR)/ails_runner: $(BIN_DIR)/ails_runner.o $(UTIL_OBJS) $(MT_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(HIGHS_LIB) $(GUROBI_LIB) \
	    -Wl,-rpath,$(GUROBI_HOME)/lib

# -------------------- Ensure bin exists --------------------
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# -------------------- Clean --------------------
clean:
	rm -rf $(BIN_DIR)/*
#include "../util/builder.hpp"
#include "../util/rvnd.hpp"
#include "../util/bppc.hpp"
#include <iostream>

int main() {
    BPPCInstance inst = readInstance("../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt");
    double alpha = 0.2;
    int k1 = 1;
    int k2 = 2;
    int k3 = 5;
    
    // -------------------- Build initial solution --------------------
    SolutionBuilder builder(inst);
    BPPCSolution sol = builder.greedy(alpha, k1, k2, k3);

    std::cout << "--- Initial Solution ---\n";
    sol.print();
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n";

    // -------------------- Run RVND --------------------
    RVND rvnd(sol, k1, k2, k3);
    rvnd.run();

    std::cout << "\n--- Improved Solution (RVND) ---\n";
    sol.print();  // RVND modifies solution in-place
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n";

    return 0;
}
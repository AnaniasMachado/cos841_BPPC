#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/bppc.hpp"
#include <iostream>

int main() {
    BPPCInstance inst = readInstance("../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt");
    double alpha = 0.2;
    int k1 = 1;
    int k2 = 2;
    int k3 = 5;
    
    // Build an initial solution
    SolutionBuilder builder(inst);
    // BPPCSolution sol = builder.MFFD();
    // BPPCSolution sol = builder.randomFeasible();
    BPPCSolution sol = builder.greedy(alpha, k1, k2, k3);

    std::cout << "--- Initial Solution ---\n";
    sol.print();
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n";

    // Run RVND local search
    LocalSearchRVND rvnd(sol, k1, k2, k3);
    rvnd.run();

    BPPCSolution improved = rvnd.getSolution();
    std::cout << "\n--- Improved Solution (RVND) ---\n";
    improved.print();
    std::cout << "Objective: " << improved.computeObjective(k1, k2, k3) << "\n";

    return 0;
}
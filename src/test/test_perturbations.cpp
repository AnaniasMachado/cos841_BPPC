#include "../util/builder.hpp"
#include "../util/perturbations.hpp"
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

    std::cout << "Initial solution:\n";
    sol.print();
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n\n";

    Perturbations p;

    // ---- Relocate ----
    BPPCSolution s1 = sol;
    p.relocateK(s1, 3);
    std::cout << "After relocateK:\n";
    std::cout << "Objective: " << s1.computeObjective(k1, k2, k3) << "\n\n";

    // ---- Exchange ----
    BPPCSolution s2 = sol;
    p.exchangeK(s2, 3);
    std::cout << "After exchangeK:\n";
    std::cout << "Objective: " << s2.computeObjective(k1, k2, k3) << "\n\n";

    // ---- Merge ----
    BPPCSolution s3 = sol;
    p.merge(s3);
    std::cout << "After merge:\n";
    std::cout << "Objective: " << s3.computeObjective(k1, k2, k3) << "\n\n";

    // ---- Split ----
    BPPCSolution s4 = sol;
    p.split(s4);
    std::cout << "After split:\n";
    std::cout << "Objective: " << s4.computeObjective(k1, k2, k3) << "\n\n";

    return 0;
}
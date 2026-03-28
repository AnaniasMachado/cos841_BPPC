#include "../util/bppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/perturbations.hpp"
#include "../metaheuristic/ails.hpp"

#include <iostream>
#include <chrono>

int main() {
    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance("../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt");
    int k1 = 1;
    int k2 = 2;
    int k3 = 5;

    // AILS parameters
    int max_iterations = 500;
    int max_no_improve = 10;

    // QRVND parameters
    bool useQRVND = true;       // true -> use QRVND, false -> use RVND
    double alpha = 0.5;         // learning rate
    double gamma = 0.9;         // discount factor
    double epsilon = 0.02;       // epsilon-greedy

    std::cout << "===== INSTANCE STATISTICS =====\n";
    inst.printStatistics();
    std::cout << "\n";

    // -------------------- Run AILS --------------------
    auto start = std::chrono::high_resolution_clock::now();

    AILS ails(inst, k1, k2, k3,
              max_iterations, max_no_improve,
              useQRVND,
              alpha, gamma, epsilon);

    BPPCSolution best = ails.run();

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    // -------------------- Results --------------------
    std::cout << "===== AILS RESULT =====\n";
    best.printStatistics(k1, k2, k3);
    std::cout << "Execution time: " << elapsed << " seconds\n";

    return 0;
}
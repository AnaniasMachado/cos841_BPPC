#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/bppc.hpp"
#include "../util/perturbations.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>

int main() {
    using clock = std::chrono::high_resolution_clock;

    // -------------------- Instance path --------------------
    std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_3_8_9.txt";

    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance(path);

    int k1 = 1;
    int k2 = 2;
    int k3 = 5;
    ImprovementType improvement = ImprovementType::BI;

    // -------------------- Build initial solution --------------------
    SolutionBuilder builder(inst);
    double beta = 0.2;
    BPPCSolution sol = builder.greedy(beta, k1, k2, k3);

    std::cout << "--- Initial Solution ---\n";
    std::cout << "Bins: " << sol.binsUsed() << "\n";
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n\n";

    Perturbations pert;
    LocalSearch ls(sol, improvement, k1, k2, k3);

    std::cout << std::fixed << std::setprecision(6);

    // ======================================================
    // -------------------- ASSIGNMENT SWEEP -----------------
    // ======================================================

    std::cout << "===== ASSIGNMENT SWEEP =====\n";
    std::cout << "k\tTime(s)\t\tBefore\tAfter\tImproved\n";

    int N = inst.N;

    // Generate k values from 1% to 20% of N
    std::vector<int> k_values;
    for (int p = 1; p <= 20; ++p) {
        int k = std::max(1, (p * N) / 100);
        k_values.push_back(k);
    }

    for (int k : k_values) {

        // Work on a copy of the original solution
        BPPCSolution temp = sol;
        // pert.exchangeK(temp, k);
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.assignment(k);
        auto end = clock::now();

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << k << "\t"
                  << time << "\t"
                  << before << "\t"
                  << after << "\t"
                  << improved << "\n";
    }

    std::cout << "\n";

    return 0;
}
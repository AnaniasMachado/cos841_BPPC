#include "../util/bppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"

#include <iostream>
#include <chrono>

int main() {
    // -------------------- Instance path --------------------
    // std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt";
    std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_4_8_9.txt";

    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance(path);

    // Objective weights (used only for greedy)
    int k1 = 1;
    int k2 = 2;
    int k3 = 5;

    // Greedy parameter
    double alpha = 0.3;

    std::cout << "===== INSTANCE STATISTICS =====\n";
    inst.printStatistics();
    std::cout << "\n";

    SolutionBuilder builder(inst);

    // ======================================================
    // -------------------- MFFD -----------------------------
    // ======================================================
    {
        auto start = std::chrono::high_resolution_clock::now();

        BPPCSolution sol = builder.MFFD();

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();

        std::cout << "===== MFFD =====\n";
        sol.printStatistics(k1, k2, k3);
        std::cout << "Time: " << elapsed << " seconds\n\n";
    }

    // ======================================================
    // -------------------- RANDOM ---------------------------
    // ======================================================
    {
        auto start = std::chrono::high_resolution_clock::now();

        BPPCSolution sol = builder.randomFeasible();

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();

        std::cout << "===== RANDOM FEASIBLE =====\n";
        sol.printStatistics(k1, k2, k3);
        std::cout << "Time: " << elapsed << " seconds\n\n";
    }

    // ======================================================
    // -------------------- GREEDY ---------------------------
    // ======================================================
    {
        auto start = std::chrono::high_resolution_clock::now();

        BPPCSolution sol = builder.greedy(alpha, k1, k2, k3);

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();

        std::cout << "===== GREEDY =====\n";
        sol.printStatistics(k1, k2, k3);
        std::cout << "Time: " << elapsed << " seconds\n\n";
    }

    return 0;
}
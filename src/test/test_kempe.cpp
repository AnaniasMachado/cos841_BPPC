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
    std::string path = "../instances/BPPC_test_instances/BPPC/ua/BPWC_4_8_9.txt";

    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance(path);

    int k1 = 226;
    int k2 = 423;
    int k3 = 250;
    ImprovementType improvement = ImprovementType::BI;

    // -------------------- Build initial solution --------------------
    SolutionBuilder builder(inst);
    double beta = 0.2;
    BPPCSolution sol = builder.greedy(beta, k1, k2, k3);
    // BPPCSolution sol = builder.MFFD();

    std::cout << "--- Initial Solution ---\n";
    std::cout << "Bins: " << sol.binsUsed() << "\n";
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n\n";

    LocalSearch ls(sol, improvement, k1, k2, k3);

    std::cout << std::fixed << std::setprecision(6);

    const int K_TIMES = 20;

    // ======================================================
    // -------------------- KEMPE CHAIN TEST -----------------
    // ======================================================
    std::cout << "===== KEMPE CHAIN =====\n";
    std::cout << "Iter\tTime(s)\tBefore\tAfter\tImproved\n";

    {
        BPPCSolution temp = sol;   // same starting solution
        ls.setSolution(temp);

        for (int it = 0; it < K_TIMES; it++) {

            int before = temp.computeObjective(k1, k2, k3);

            auto start = clock::now();
            bool improved = ls.kempeChain();
            auto end = clock::now();

            int after = temp.computeObjective(k1, k2, k3);
            double time = std::chrono::duration<double>(end - start).count();

            std::cout << it << "\t"
                      << time << "\t"
                      << before << "\t"
                      << after << "\t"
                      << improved << "\n";

            if (!improved) break;  // stop at local minimum
        }
    }

    std::cout << "\n";

    return 0;
}
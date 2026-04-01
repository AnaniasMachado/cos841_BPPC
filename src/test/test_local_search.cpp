#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/bppc.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>

int main() {
    using clock = std::chrono::high_resolution_clock;

    // -------------------- Instance path --------------------
    // std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt";
    std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_3_8_9.txt";

    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance(path);

    double alpha = 0.3;
    int k1 = 1;
    int k2 = 2;
    int k3 = 5;

    // -------------------- Build initial solution --------------------
    SolutionBuilder builder(inst);
    BPPCSolution sol = builder.MFFD();

    std::cout << "--- Initial Solution ---\n";
    std::cout << "Bins: " << sol.binsUsed() << "\n";
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n\n";

    LocalSearch ls(sol, k1, k2, k3);

    std::cout << std::fixed << std::setprecision(6);

    // ======================================================
    // -------------------- RELOCATION -----------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.relocation();
        auto end = clock::now();

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== RELOCATION =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- EXCHANGE -------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.exchange();
        auto end = clock::now();

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EXCHANGE =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- ADD ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.add();
        auto end = clock::now();

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== ADD =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    return 0;
}
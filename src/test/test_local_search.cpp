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
    // std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_4_8_9.txt";
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

    std::cout << "--- Initial Solution ---\n";
    std::cout << "Bins: " << sol.binsUsed() << "\n";
    std::cout << "Objective: " << sol.computeObjective(k1, k2, k3) << "\n\n";

    LocalSearch ls(sol, improvement, k1, k2, k3);

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

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

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

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EXCHANGE =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- EXCHANGE 21 -------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.exchange21();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EXCHANGE 21 =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- CLASSIC -------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.classic();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== CLASSIC =====\n";
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

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== ADD =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- EJECTION ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.ejection();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EJECTION =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- EJECTION GREEDY ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.ejectionGreedy();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EJECTION GREEDY =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- EJECTION GC ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.ejectionGC();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EJECTION GC =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- EJECTION GLOBAL ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.ejectionGlobal();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EJECTION GLOBAL =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- EJECTION CHAIN ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.ejectionChain();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== EJECTION CHAIN =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- GRENADE ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.grenade();
        auto end = clock::now();

        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== GRENADE =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- ASSIGNMENT ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.assignment((int)inst.N / 5);
        auto end = clock::now();
        
        ls.updateK();
        ls.updateElite();
        ls.addToPool(temp);

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== ASSIGNMENT =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    // ======================================================
    // -------------------- SET COVERING ------------------------------
    // ======================================================
    {
        BPPCSolution temp = sol;
        ls.setSolution(temp);

        int before = temp.computeObjective(k1, k2, k3);

        auto start = clock::now();
        bool improved = ls.setCoveringBinFeasible();
        auto end = clock::now();

        int after = temp.computeObjective(k1, k2, k3);
        double time = std::chrono::duration<double>(end - start).count();

        std::cout << "===== SET COVERING =====\n";
        std::cout << "Improved: " << improved << "\n";
        std::cout << "Before: " << before << "\n";
        std::cout << "After: " << after << "\n";
        std::cout << "Time: " << time << " s\n\n";
    }

    return 0;
}
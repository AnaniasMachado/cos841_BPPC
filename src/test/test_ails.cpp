#include "../util/bppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/perturbations.hpp"
#include "../metaheuristic/ails.hpp"
#include "../util/bks.hpp"

#include <iostream>
#include <chrono>

int main() {
    // -------------------- Instance path --------------------
    std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt";

    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance(path);

    int k1 = 1;
    int k2 = 2;
    int k3 = 5;

    // AILS parameters
    int max_iterations = 500;
    int max_no_improve = 10;

    // -------------------- Builder selection --------------------
    BuilderType builder = BuilderType::GREEDY;
    double beta = 0.3;

    // QRVND parameters
    bool useQRVND = false;
    double alpha = 0.5;
    double gamma = 0.9;
    double epsilon = 0.02;

    std::cout << "===== INSTANCE STATISTICS =====\n";
    inst.printStatistics();

    // -------------------- Load BKS --------------------
    BKSLoader bks("../solutions/bks/bks_table.txt");

    std::string instance_name = extractInstanceName(path);
    std::cout << "Instance key: " << instance_name << "\n\n";
    std::cout << "\n";

    if (!bks.hasInstance(instance_name)) {
        std::cout << "Warning: BKS not found for instance\n";
    }

    // -------------------- Run AILS --------------------
    auto start = std::chrono::high_resolution_clock::now();

    AILS ails(inst, k1, k2, k3,
              max_iterations, max_no_improve,
              builder,
              beta,
              useQRVND,
              alpha, gamma, epsilon);

    BPPCSolution best = ails.run();

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    // -------------------- Results --------------------
    std::cout << "===== AILS RESULT =====\n";
    best.printStatistics(k1, k2, k3);
    std::cout << "Execution time: " << elapsed << " seconds\n";

    // -------------------- Compare with BKS --------------------
    auto res = bks.evaluate(instance_name, best.binsUsed());

    std::cout << "\n===== COMPARISON WITH BKS =====\n";
    std::cout << "BKS: " << res.bks << "\n";
    std::cout << "Gap (%): " << res.gap << "\n";
    std::cout << "Reached BKS: " << res.reach_bks << "\n";
    std::cout << "Reached OPT: " << res.reach_opt << "\n";

    if (res.bks != -1 && best.binsUsed() < res.bks) {
        std::cout << ">>> NEW BEST SOLUTION FOUND! <<<\n";
    }

    return 0;
}
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
    // std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt";
    std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_4_8_9.txt";
    // std::string path = "../instances/BPPC_test_instances/BPPC/ua/BPWC_4_8_9.txt";

    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance(path);

    int k1 = 130;
    int k2 = 136;
    int k3 = 399;

    // AILS parameters
    int max_iterations = 218;
    int max_no_improve = 24;
    AcceptanceType acceptance = AcceptanceType::BEST;
    ImprovementType improvement = ImprovementType::BI;

    // EMA parameters
    bool useEMA = true;
    double eta = 0.1;

    // -------------------- Builder selection --------------------
    BuilderType builder = BuilderType::MFFD;
    double beta = 0.2;

    // QRVND parameters
    bool useQRVND = true;
    double alpha = 0.2343;
    double gamma = 0.9488;
    double epsilon = 0.4318;

    bool verbose = true;
    double time_limit = 3600.0;

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
              acceptance, improvement,
              useEMA, eta,
              builder,
              beta,
              useQRVND,
              alpha, gamma, epsilon,
              verbose, time_limit);

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
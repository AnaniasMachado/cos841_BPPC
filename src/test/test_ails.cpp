#include "../util/bppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/perturbations.hpp"
#include "../metaheuristic/ails.hpp"
#include "../util/bks.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <limits>

int main() {
    using clock = std::chrono::high_resolution_clock;

    // -------------------- Instance path --------------------
    // std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_1_1_1.txt";
    // std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_4_8_9.txt";
    // std::string path = "../instances/BPPC_test_instances/BPPC/ua/BPWC_4_8_9.txt";
    // std::string path = "../instances/BPPC_test_instances/BPPC/d/BPWC_2_6_9.txt";
    std::string path = "../instances/BPPC_test_instances/BPPC/da/BPWC_3_8_9.txt";

    // -------------------- Load instance --------------------
    BPPCInstance inst = readInstance(path);

    int k1 = 226;
    int k2 = 423;
    int k3 = 250;

    // AILS parameters
    // int max_iterations = 384;
    // int max_no_improve = 22;
    int max_iterations = 50;
    AcceptanceType acceptance = AcceptanceType::BEST;
    ImprovementType improvement = ImprovementType::BI;

    // UCB parameters
    bool useUCB = true;
    double c = 0.5718;

    // Builder
    BuilderType builder = BuilderType::RANDOM;
    double beta = 0.2;

    // QRVND parameters
    bool useQRVND = true;
    double alpha = 0.1495;
    double gamma = 0.7455;
    double epsilon = 0.0377;

    bool verbose = true;
    double time_limit = 3600.0;

    int k_runs = 5;  // number of runs

    std::cout << "===== INSTANCE STATISTICS =====\n";
    inst.printStatistics();

    // -------------------- Load BKS --------------------
    BKSLoader bks("../solutions/bks/bks_table.txt");
    std::string instance_name = extractInstanceName(path);

    std::cout << "Instance key: " << instance_name << "\n\n";

    // -------------------- Accumulators --------------------
    std::vector<int> objs;
    std::vector<double> times;
    std::vector<double> gaps;

    // -------------------- Best over all runs --------------------
    int best_obj_global = std::numeric_limits<int>::max();
    double best_gap = std::numeric_limits<double>::max();
    BPPCSolution best_solution_global(inst.N, inst.C, inst.weights, inst.conflicts);
    int best_run = -1;
    bool has_best = false;

    // ==================== MULTIPLE RUNS ====================
    for (int run = 1; run <= k_runs; run++) {

        std::cout << "\n===== RUN " << run << " =====\n";

        auto start = clock::now();

        AILS ails(inst, k1, k2, k3,
                  max_iterations,
                  acceptance, improvement,
                  useUCB, c,
                  builder,
                  beta,
                  useQRVND,
                  alpha, gamma, epsilon,
                  verbose, time_limit);

        BPPCSolution best = ails.run();

        auto end = clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();

        int obj = best.isFeasible()
                ? best.binsUsed()
                : std::numeric_limits<int>::max();

        std::cout << "===== RESULT =====\n";
        best.printStatistics(k1, k2, k3);
        std::cout << "Execution time: " << elapsed << " seconds\n";

        // -------------------- Compare with BKS --------------------
        auto res = bks.evaluate(instance_name, obj);

        std::cout << "\n===== COMPARISON WITH BKS =====\n";
        std::cout << "BKS: " << res.bks << "\n";
        std::cout << "Gap (%): " << res.gap << "\n";
        std::cout << "Reached BKS: " << res.reach_bks << "\n";
        std::cout << "Reached OPT: " << res.reach_opt << "\n";

        if (res.bks != -1 && obj < res.bks) {
            std::cout << ">>> NEW BEST SOLUTION FOUND! <<<\n";
        }

        // -------------------- Store stats --------------------
        objs.push_back(obj);
        times.push_back(elapsed);
        gaps.push_back(res.gap);

        // -------------------- Update global best --------------------
        if (!has_best || obj < best_obj_global) {
            best_obj_global = obj;
            best_gap = res.gap;
            best_solution_global = best;
            best_run = run;
            has_best = true;
        }
    }

    // ==================== AVERAGES ====================
    double avg_obj = std::accumulate(objs.begin(), objs.end(), 0.0) / k_runs;
    double avg_time = std::accumulate(times.begin(), times.end(), 0.0) / k_runs;
    double avg_gap = std::accumulate(gaps.begin(), gaps.end(), 0.0) / k_runs;

    std::cout << "\n\n===== FINAL AVERAGES (" << k_runs << " runs) =====\n";
    std::cout << "Average bins: " << avg_obj << "\n";
    std::cout << "Average time: " << avg_time << " seconds\n";
    std::cout << "Average gap (%): " << avg_gap << "\n";

    // ==================== BEST OVER ALL RUNS ====================
    std::cout << "\n===== BEST OVER ALL RUNS =====\n";
    std::cout << "Best run: " << best_run << "\n";
    best_solution_global.printStatistics(k1, k2, k3);
    std::cout << "Best gap (%): " << best_gap << "\n";

    return 0;
}
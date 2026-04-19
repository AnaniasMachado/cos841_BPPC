#include "../util/bppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/perturbations.hpp"
#include "../metaheuristic/ails.hpp"
#include "../util/bks.hpp"

#include "../util/experiment_runner.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <limits>
#include <algorithm>
#include <ctime>
#include <iomanip>

void printCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);

    std::cout << "Current OS time: "
              << std::put_time(tm, "%Y-%m-%d %H:%M:%S")
              << "\n";
}

int main() {
    using clock = std::chrono::high_resolution_clock;

    std::string instances_root = "../instances/BPPC_test_instances/BPPC";

    // -------------------- Initialize runner --------------------
    ExperimentRunner runner(instances_root);

    // -------------------- Load filtered instances --------------------
    std::vector<std::string> instance_paths = runner.getAllInstancePaths();

    if (instance_paths.empty()) {
        std::cerr << "No instances found.\n";
        return 1;
    }

    std::sort(instance_paths.begin(), instance_paths.end());

    // -------------------- Load BKS --------------------
    BKSLoader bks("../solutions/bks/bks_table.txt");

    // -------------------- AILS parameters --------------------
    int k1 = 226, k2 = 423, k3 = 250;

    int max_iterations = 384;
    int max_no_improve = 22;
    AcceptanceType acceptance = AcceptanceType::BEST;
    ImprovementType improvement = ImprovementType::BI;

    bool useUCB = true;
    double c = 0.5718;

    BuilderType builder = BuilderType::RANDOM;
    double beta = 0.2;

    bool useQRVND = true;
    double alpha = 0.1495, gamma = 0.7455, epsilon = 0.0377;

    bool verbose = false;
    double time_limit = 3600.0;

    int k_runs = 3;

    // ==================== LOOP OVER INSTANCES ====================
    for (const auto& path : instance_paths) {

        std::cout << "\n========================================\n";
        std::cout << "Instance: " << path << "\n";

        BPPCInstance inst = readInstance(path);

        if (inst.N == 0) {
            std::cerr << "Invalid instance, skipping.\n";
            continue;
        }

        // -------------------- Naming --------------------
        std::string instance_name = extractInstanceName(path);

        std::string safe_name = instance_name;
        std::replace(safe_name.begin(), safe_name.end(), '/', '_');

        // -------------------- Output folder --------------------
        std::string rel_path = runner.extractRelativePath(path);
        std::string output_folder = runner.buildOutputFolder(rel_path);

        runner.ensureDirectory(output_folder);

        // -------------------- Resume logic --------------------
        int start_run = runner.countExistingRuns(output_folder, safe_name);

        if (start_run >= k_runs) {
            std::cout << "Already completed ("
                      << start_run << "/" << k_runs << "), skipping.\n";
            continue;
        }

        std::cout << "Resuming from run " << start_run
                  << " (" << k_runs << " total)\n";

        // ==================== RUNS ====================
        for (int run = start_run; run < k_runs; run++) {

            std::cout << "\nRun " << run << "\n";

            // OS TIME LOG HERE
            printCurrentTime();

            auto start = clock::now();

            AILS ails(inst, k1, k2, k3,
                      max_iterations, max_no_improve,
                      acceptance, improvement,
                      useUCB, c,
                      builder, beta,
                      useQRVND,
                      alpha, gamma, epsilon,
                      verbose, time_limit);

            BPPCSolution best = ails.run();

            auto end = clock::now();
            double elapsed = std::chrono::duration<double>(end - start).count();

            int obj = best.isFeasible()
                    ? best.binsUsed()
                    : std::numeric_limits<int>::max();

            bool feasible = best.isFeasible();

            std::string filename =
                output_folder + "/sol_" +
                std::to_string(run) + "_" +
                safe_name + ".txt";

            runner.saveSolutionToFile(filename, best, elapsed, obj, feasible);

            auto res = bks.evaluate(instance_name, obj);

            std::cout << "Obj: " << obj
                      << " | Gap: " << res.gap
                      << " | Time: " << elapsed << "\n";
        }
    }

    return 0;
}
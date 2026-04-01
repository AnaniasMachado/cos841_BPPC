#include "bppc.hpp"
#include "solution.hpp"
#include "builder.hpp"
#include "../metaheuristic/ails.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <string>
#include <climits>

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " instance_file K1 K2 K3\n";
        return 1;
    }

    std::string instance_file = argv[1];
    int K1 = std::stoi(argv[2]);
    int K2 = std::stoi(argv[3]);
    int K3 = std::stoi(argv[4]);

    // Fixed AILS parameters
    BuilderType builder = BuilderType::MFFD;
    double beta = 0.3;
    bool useQRVND = false;
    double alpha = 0.1;
    double gamma = 0.9;
    double epsilon = 0.1;
    int max_iterations = 500;
    int max_no_improve = 10;

    // ---- Read BPPC instance ----
    std::ifstream fin(instance_file);
    if (!fin) {
        std::cerr << "Cannot open instance file: " << instance_file << "\n";
        return 1;
    }

    int N, C;
    fin >> N >> C;

    std::vector<int> weights(N, 0);
    std::vector<std::unordered_set<int>> conflicts(N);

    std::string line;
    std::getline(fin, line); // consume rest of first line

    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        int item_index, w;
        ss >> item_index >> w;
        item_index--;  // convert to 0-based
        weights[item_index] = w;

        int conflict_item;
        while (ss >> conflict_item) {
            conflict_item--;  // convert to 0-based
            conflicts[item_index].insert(conflict_item);
            conflicts[conflict_item].insert(item_index); // symmetric
        }
    }
    fin.close();

    BPPCInstance inst{N, C, weights, conflicts};
    // --------------------------------------

    // Run AILS
    AILS ails(inst, K1, K2, K3, max_iterations, max_no_improve,
              builder, beta, useQRVND, alpha, gamma, epsilon);

    BPPCSolution sol = ails.run();

    // Print only the objective value
    if (sol.isFeasible()) {
        std::cout << sol.binsUsed() << std::endl;
    } else {
        std::cout << INT_MAX << std::endl;
    }

    return 0;
}
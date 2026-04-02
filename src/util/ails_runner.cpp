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

// -------------------- Helper: parse builder --------------------
BuilderType parseBuilder(const std::string& s) {
    if (s == "MFFD") return BuilderType::MFFD;
    if (s == "RANDOM") return BuilderType::RANDOM;
    if (s == "GREEDY") return BuilderType::GREEDY;
    return BuilderType::MFFD; // default
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " instance_file K1 K2 K3 [options]\n";
        return 1;
    }

    std::string instance_file = argv[1];
    int K1 = std::stoi(argv[2]);
    int K2 = std::stoi(argv[3]);
    int K3 = std::stoi(argv[4]);

    // -------------------- Default AILS parameters --------------------
    BuilderType builder = BuilderType::MFFD;
    double beta = 0.3;
    bool useQRVND = true;
    double alpha = 0.1;
    double gamma = 0.9;
    double epsilon = 0.1;
    int max_iterations = 50;
    int max_no_improve = 5;
    bool verbose = false;

    // -------------------- Parse optional arguments --------------------
    for (int i = 5; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--max_it" && i + 1 < argc) {
            max_iterations = std::stoi(argv[++i]);
        } 
        else if (arg == "--max_no_imp" && i + 1 < argc) {
            max_no_improve = std::stoi(argv[++i]);
        } 
        else if (arg == "--builder" && i + 1 < argc) {
            builder = parseBuilder(argv[++i]);
        } 
        else if (arg == "--beta" && i + 1 < argc) {
            beta = std::stod(argv[++i]);
        } 
        else if (arg == "--use_qrvnd" && i + 1 < argc) {
            useQRVND = std::stoi(argv[++i]) != 0;
        } 
        else if (arg == "--alpha" && i + 1 < argc) {
            alpha = std::stod(argv[++i]);
        } 
        else if (arg == "--gamma" && i + 1 < argc) {
            gamma = std::stod(argv[++i]);
        } 
        else if (arg == "--epsilon" && i + 1 < argc) {
            epsilon = std::stod(argv[++i]);
        }
        else if (arg == "--verbose") {   // <-- NEW
            verbose = true;
        }
    }

    // -------------------- Read BPPC instance --------------------
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
    std::getline(fin, line);

    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        int item_index, w;
        ss >> item_index >> w;
        item_index--;
        weights[item_index] = w;

        int conflict_item;
        while (ss >> conflict_item) {
            conflict_item--;
            conflicts[item_index].insert(conflict_item);
            conflicts[conflict_item].insert(item_index);
        }
    }
    fin.close();

    BPPCInstance inst{N, C, weights, conflicts};

    // -------------------- Run AILS --------------------
    AILS ails(inst, K1, K2, K3,
          max_iterations, max_no_improve,
          builder, beta, useQRVND,
          alpha, gamma, epsilon,
          verbose);

    BPPCSolution sol = ails.run();

    // -------------------- Output --------------------
    if (sol.isFeasible()) {
        std::cout << sol.binsUsed() << std::endl;
    } else {
        std::cout << INT_MAX << std::endl;
    }

    return 0;
}
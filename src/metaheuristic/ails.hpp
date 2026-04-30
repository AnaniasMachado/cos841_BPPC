#ifndef AILS_HPP
#define AILS_HPP

#include "../util/bppc.hpp"
#include "../util/builder.hpp"
#include "../util/perturbations.hpp"
#include "../util/qrvnd.hpp"
#include "../util/rvnd.hpp"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>
#include <limits>

// -------------------- Builder Selection --------------------
enum class BuilderType {
    MFFD,
    RANDOM,
    GREEDY
};

// -------------------- Acceptance Selection --------------------
enum class AcceptanceType {
    BEST,
    ITERATIVE,
    RW
};

class AILS {
public:
    AILS(const BPPCInstance& instance,
         int k1, int k2, int k3,
         int max_it,
         AcceptanceType acceptance_type = AcceptanceType::BEST,
         ImprovementType improvement_type = ImprovementType::BI,
         bool use_ucb = true,
         double c = 0.25,
         BuilderType builder_type = BuilderType::MFFD,
         double beta_ = 0.3,
         bool use_qrvnd = true,
         double alpha_q = 0.1,
         double gamma_q = 0.9,
         double epsilon_q = 0.1,
         bool verbose = false,
         double time_limit = 3600.0);

    BPPCSolution run();

private:
    const BPPCInstance& inst;

    int K1, K2, K3;
    int max_iterations;
    AcceptanceType acceptance_type;
    ImprovementType improvement_type;

    // UCB parameters
    std::mt19937 rng;
    std::uniform_int_distribution<> dist;
    bool useUCB;
    double c;
    std::vector<int> perturbation_count;
    std::vector<int> perturbation_success;

    // Builder parameters
    BuilderType builder_type;
    double beta;

    // QRVND parameters
    bool useQRVND;
    double alpha, gamma, epsilon;

    bool verbose;
    double time_limit;

    int computeLowerBound(const BPPCSolution& s) const;

    int computeK(PerturbationType perturbation_type, BPPCSolution& sol, int iter);

    PerturbationType getPerturbationType(int idx);

    PerturbationType selectPerturbation(int iter);

    void applyPerturbation(PerturbationType perturbation_type,
                            BPPCSolution& sol, int iter);

    void updateUCB(PerturbationType perturbation_type, bool reward);
};

#endif
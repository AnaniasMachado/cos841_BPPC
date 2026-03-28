#ifndef AILS_HPP
#define AILS_HPP

#include "../util/bppc.hpp"
#include "../util/builder.hpp"
#include "../util/perturbations.hpp"
#include "../util/qrvnd.hpp"
#include "../util/rvnd.hpp"

#include <random>
#include <vector>

class AILS {
public:
    AILS(const BPPCInstance& instance,
         int k1, int k2, int k3,
         int max_it,
         int max_no_imp,
         bool use_qrvnd = false,
         double alpha_q = 0.1,
         double gamma_q = 0.9,
         double epsilon_q = 0.1);

    BPPCSolution run();

private:
    const BPPCInstance& inst;

    int K1, K2, K3;
    int max_iterations;
    int max_no_improve;

    // QRVND parameters
    bool useQRVND;
    double alpha, gamma, epsilon;

    std::mt19937 rng;
    std::vector<double> weights;

    int computeK(int current_obj, int best_obj,
                 int no_improve, int iter);

    int selectPerturbation();

    void applyPerturbation(int idx, BPPCSolution& sol,
                           int current_obj, int best_obj,
                           int no_improve, int iter);
};

#endif
#ifndef AILS_HPP
#define AILS_HPP

#include "../util/bppc.hpp"
#include "../util/solution.hpp"
#include "../util/builder.hpp"
#include "../util/local_search.hpp"
#include "../util/perturbations.hpp"
#include <vector>
#include <random>

class AILS {
public:
    AILS(const BPPCInstance& instance,
         int k1, int k2, int k3,
         int max_it,
         int max_no_imp);

    BPPCSolution run();

private:
    const BPPCInstance& inst;
    int K1, K2, K3;
    int max_iterations;
    int max_no_improve;

    std::mt19937 rng;
    std::vector<double> weights; // adaptive perturbation probabilities

    int selectPerturbation();
    void applyPerturbation(int idx, BPPCSolution& sol,
                           int current_obj,
                           int best_obj,
                           int no_improve,
                           int iter);

    int computeK(int current_obj, int best_obj, int no_improve, int iter);
};

#endif
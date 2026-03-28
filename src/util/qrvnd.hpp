#ifndef QRVND_HPP
#define QRVND_HPP

#include "local_search.hpp"
#include <vector>
#include <random>

class QRVND {
private:
    BPPCSolution& sol;
    LocalSearch ls;

    int K1,K2,K3;

    double alpha, gamma, epsilon;
    int max_iterations, max_no_improve;

    std::vector<std::vector<int>> perms;
    std::vector<std::vector<double>> Q;

    std::mt19937 rng;

    int selectPermutation(int current_p);
    bool applyOrder(const std::vector<int>& order);

public:
    QRVND(BPPCSolution& solution, int k1, int k2, int k3,
          double a, double g, double e,
          int max_it, int max_no_imp);

    void run();
};

#endif
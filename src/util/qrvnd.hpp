#ifndef QRVND_HPP
#define QRVND_HPP

#include "local_search.hpp"
#include <vector>
#include <random>

class QRVND {
private:
    BPPCSolution* sol;
    LocalSearch ls;

    int K1,K2,K3;

    double alpha, gamma, epsilon;

    std::vector<std::vector<int>> perms;
    std::vector<std::vector<double>> Q;

    std::mt19937 rng;
    std::uniform_real_distribution<double> dist01;

    bool initialized;
    int current_p;

    int selectPermutation(int current_p);
    bool applyOrder(const std::vector<int>& order);

public:
    QRVND(BPPCSolution& solution, int k1, int k2, int k3,
          double a, double g, double e);

    void run();
    void setSolution(BPPCSolution& solution);
};

#endif
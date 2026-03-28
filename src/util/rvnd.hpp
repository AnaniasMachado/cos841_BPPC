#ifndef RVND_HPP
#define RVND_HPP

#include "local_search.hpp"
#include <vector>
#include <random>

class RVND {
private:
    BPPCSolution* sol;
    LocalSearch ls;
    int K1, K2, K3;
    std::mt19937 rng;

public:
    RVND(BPPCSolution& solution, int k1, int k2, int k3);

    void run();
    void setSolution(BPPCSolution& solution);
};

#endif
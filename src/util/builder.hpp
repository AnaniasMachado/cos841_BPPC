#ifndef SOLUTION_BUILDER_HPP
#define SOLUTION_BUILDER_HPP

#include "bppc.hpp"
#include "solution.hpp"
#include <vector>
#include <random>

class SolutionBuilder {
public:
    SolutionBuilder(const BPPCInstance& instance);

    // Construct a solution using Modified First Fit Decreasing
    BPPCSolution MFFD();

    // Construct a random feasible solution
    BPPCSolution randomFeasible();

    // Construct a greedy solution
    BPPCSolution greedy(double alpha, int k1, int k2, int k3);

private:
    const BPPCInstance& inst;
    std::mt19937 rng; // random number generator
};

#endif
#ifndef PERTURBATIONS_HPP
#define PERTURBATIONS_HPP

#include "solution.hpp"
#include <random>

// -------------------- Perturbation Selection --------------------
enum class PerturbationType {
    RELOCATEK,
    EXCHANGEK,
    MERGE,
    MERGEK,
    SPLIT,
    SPLITK
};

class Perturbations {
private:
    std::mt19937 rng;

public:
    Perturbations();

    // Relocate k random items to random bins
    void relocateK(BPPCSolution& sol, int k);

    // Exchange k random pairs between bins
    void exchangeK(BPPCSolution& sol, int k);

    // Merge two random bins
    void merge(BPPCSolution& sol);

    // Split one random bin into others
    void split(BPPCSolution& sol);

    // Merge k random pairs of random bins
    void mergeK(BPPCSolution& sol, int k);

    // Split k random bin into others
    void splitK(BPPCSolution& sol, int k);
};

#endif
#ifndef ILS_BPPC_HPP
#define ILS_BPPC_HPP

#include "solution.hpp"
#include "local_search.hpp"
#include <random>
#include <vector>

class ILS {
public:
    struct Params {
        int max_shakes = 10;   // max number of consecutive shakes without improvement
        int n_ls = 10;         // number of LS macro-iterations per inner loop step
        int n_sc = 5;          // call set covering every n_sc non-improving shakes
        int n_assign = 10;     // parameter for assignment neighborhood
        int shake_size = 10;   // number of relocated items during shaking
        unsigned seed = std::random_device{}();
    };

    ILS(const BPPCSolution& initial_solution,
        ImprovementType improvement_type,
        int k1, int k2, int k3,
        const Params& params,
        bool verbose = false,
        double time_limit = 3600.0,
        int max_outer_iterations = 100000);

    BPPCSolution run();

private:
    Params params_;
    int K1_;
    int K2_;
    int K3_;

    std::mt19937 rng_;
    BPPCSolution s_best_;
    LocalSearch ls_;

    bool verbose = false;
    double time_limit = 3600.0;
    int max_outer_iterations = 100000;

    int computeLowerBound(const BPPCSolution& s) const;
    int cost(const BPPCSolution& s) const;

    BPPCSolution reduceNbBins(const BPPCSolution& s);
    void localSearchPhase(BPPCSolution& s);
    void maybeSetCovering(BPPCSolution& s);
    void updatePoolFromLocalMinimum(BPPCSolution& s);
    BPPCSolution shaking(const BPPCSolution& s);

    std::vector<int> collectAllItems(const BPPCSolution& s) const;
    std::vector<int> sampleWithoutReplacement(const std::vector<int>& source, int k);
};

#endif
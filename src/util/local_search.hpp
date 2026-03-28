#ifndef LOCAL_SEARCH_HPP
#define LOCAL_SEARCH_HPP

#include "solution.hpp"
#include <vector>

class LocalSearchRVND {
public:
    LocalSearchRVND(BPPCSolution& solution, int k_bins = 1, int k_excess = 1, int k_conflicts = 1);

    // Run the RVND local search
    void run();

    // Get the improved solution
    BPPCSolution getSolution() const;

private:
    BPPCSolution sol;
    int K1, K2, K3;

    // Neighborhood operators
    bool relocation();
    bool exchange();
    bool add();

    // Utility: compute objective value for solution
    int computeObjective(const BPPCSolution& s) const;

    // Shuffle neighborhood list
    void shuffleNeighborhoods(std::vector<int>& neighborhoods);
};

#endif
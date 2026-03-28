#ifndef LOCAL_SEARCH_HPP
#define LOCAL_SEARCH_HPP

#include "solution.hpp"

class LocalSearch {
private:
    BPPCSolution& sol;
    int K1, K2, K3;

    int computeObjective(const BPPCSolution& s) const;

public:
    LocalSearch(BPPCSolution& solution, int k1, int k2, int k3);

    bool relocation();
    bool exchange();
    bool add();
};

#endif
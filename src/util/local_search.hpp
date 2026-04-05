#ifndef LOCAL_SEARCH_HPP
#define LOCAL_SEARCH_HPP

#include "solution.hpp"

enum class ImprovementType {
    FI,
    BI
};

class LocalSearch {
private:
    BPPCSolution* sol;
    int K1, K2, K3;
    ImprovementType improvement_type;

    int computeObjective(const BPPCSolution& s) const;

public:
    LocalSearch(BPPCSolution& solution, ImprovementType improvement, int k1, int k2, int k3);

    bool relocation();
    bool exchange();
    bool add();
    bool ejection();

    void setSolution(BPPCSolution& solution);
};

#endif
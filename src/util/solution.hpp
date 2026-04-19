#ifndef BPPC_SOLUTION_HPP
#define BPPC_SOLUTION_HPP

#include <vector>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstdint>

class BPPCSolution {
    friend class SolutionBuilder;
    friend class LocalSearch;
    friend class RVND;
    friend class QRVND;
    friend class Perturbations;
    friend class AILS;
    friend class ExperimentRunner;
public:
    using Bin = std::vector<int>;
    using Bitset = std::vector<uint64_t>;

    BPPCSolution(int N_items, int bin_capacity,
                const std::vector<int>& item_weights,
                const std::vector<Bitset>& item_conflicts);

    // Get number of conflits of an item in a bin
    int itemConflicts(int item, int bin_index) const;

    // Add an item to a bin
    void addItemToBin(int item, int bin_index);

    // Remove an item from a bin
    void removeItemFromBin(int item, int bin_index);

    // Move item from one bin to another
    void moveItem(int item, int from_bin, int to_bin);

    // Swap items between two bins
    void swapItems(int bin1, int idx1, int bin2, int idx2);

    // Rebuild solution from bins
    void rebuildSolutionFromBins(const std::vector<std::vector<int>>& new_bins);

    // Compute objective function with given weights
    int computeObjective(int k1, int k2, int k3) const;

    // Print solution
    void print() const;

    // Print statistics
    void printStatistics(int k1, int k2, int k3) const;

    // Sanity check
    void sanityCheck() const;

    // Get number of bins used
    int binsUsed() const;

    // Remove empty bins
    void removeEmptyBins();

    // Check if a solution is feasible
    bool isFeasible() const;

    // Get excess
    int getExcess() const;

    // Get conflicts
    int getConflicts() const;

    // -------------------- Delta evaluations --------------------
    int deltaAdd(int item, int bin_index, int k1, int k2, int k3) const;
    int deltaRemove(int item, int bin_index, int k1, int k2, int k3) const;
    int deltaMove(int item, int from_bin, int to_bin, int k1, int k2, int k3) const;
    int deltaSwap(int bin1, int idx1, int bin2, int idx2, int k1, int k2, int k3) const;
    int deltaSwap21(int bin1, int idx1a, int idx1b,
                    int bin2, int idx2, int k1, int k2, int k3) const;
    int deltaAddMultiple(const std::vector<int>& items, int bin_index,
                        int k1, int k2, int k3) const;
    int deltaRemoveMultiple(const std::vector<int>& items, int bin_index,
                        int k1, int k2, int k3) const;
    int deltaSwapSubset(int binA, const std::vector<int>& S,
                        int binB, const std::vector<int>& R,
                        int k1, int k2, int k3) const;

private:
    int N; // number of items
    int C; // bin capacity
    std::vector<int> weights; // weights[i]
    std::vector<Bitset> conflicts; // conflicts[i]

    std::vector<Bin> bins; // vector of bins
    std::vector<int> bin_loads; // current load of each bin
    std::vector<int> bin_conflicts; // number of conflicts of each bin
    std::unordered_set<int> bad_bins; // bins with excess weight or conflicts
    std::vector<int> item_bin; // bin where each item is

    int bins_used;
    int excess_weight;
    int conflicts_count;

    // Helper functions
    int computeExcessWeight() const;
    int computeConflicts() const;

    inline bool hasConflict(int a, int b) const {
        return conflicts[a][b >> 6] & (1ULL << (b & 63));
    }
};

#endif
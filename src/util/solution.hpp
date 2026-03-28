#ifndef BPPC_SOLUTION_HPP
#define BPPC_SOLUTION_HPP

#include <vector>
#include <unordered_set>

class BPPCSolution {
    friend class SolutionBuilder;
    friend class LocalSearchRVND;
    friend class Perturbations;
    friend class AILS;
public:
    using Bin = std::vector<int>;

    BPPCSolution(int N_items, int bin_capacity, 
                 const std::vector<int>& item_weights,
                 const std::vector<std::unordered_set<int>>& conflicts);

    // Add an item to a bin
    void addItemToBin(int item, int bin_index);

    // Move item from one bin to another
    void moveItem(int item, int from_bin, int to_bin);

    // Swap items between two bins
    void swapItems(int bin1, int idx1, int bin2, int idx2);

    // Compute objective function with given weights
    int computeObjective(int k1, int k2, int k3) const;

    // Print solution
    void print() const;

    // Print statistics
    void printStatistics(int k1, int k2, int k3) const;

    // Get number of bins used
    int binsUsed() const;

    // Remove empty bins
    void removeEmptyBins();

    // Get excess
    int getExcess() const;

    // Get conflicts
    int getConflicts() const;

private:
    int N; // number of items
    int C; // bin capacity
    std::vector<int> weights; // weights[i]
    std::vector<std::unordered_set<int>> conflicts; // conflicts[i]

    std::vector<Bin> bins; // vector of bins
    std::vector<int> bin_loads; // current load of each bin

    // Helper functions
    int computeExcessWeight() const;
    int computeConflicts() const;
};

#endif
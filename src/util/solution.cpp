#include "solution.hpp"
#include <iostream>
#include <algorithm>

BPPCSolution::BPPCSolution(int N_items, int bin_capacity, 
                           const std::vector<int>& item_weights,
                           const std::vector<std::unordered_set<int>>& item_conflicts)
    : N(N_items), C(bin_capacity), weights(item_weights), conflicts(item_conflicts)
{
    bins.clear();
    bin_loads.clear();
}

void BPPCSolution::addItemToBin(int item, int bin_index) {
    if (bin_index >= bins.size()) {
        bins.resize(bin_index + 1);
        bin_loads.resize(bin_index + 1, 0);
    }
    bins[bin_index].push_back(item);
    bin_loads[bin_index] += weights[item];
}

void BPPCSolution::moveItem(int item, int from_bin, int to_bin) {
    auto& bin_from = bins[from_bin];
    auto it = std::find(bin_from.begin(), bin_from.end(), item);
    if (it != bin_from.end()) {
        bin_from.erase(it);
        bin_loads[from_bin] -= weights[item];
    }
    addItemToBin(item, to_bin);
}

void BPPCSolution::swapItems(int bin1, int idx1, int bin2, int idx2) {
    int item1 = bins[bin1][idx1];
    int item2 = bins[bin2][idx2];

    bins[bin1][idx1] = item2;
    bins[bin2][idx2] = item1;

    bin_loads[bin1] = bin_loads[bin1] - weights[item1] + weights[item2];
    bin_loads[bin2] = bin_loads[bin2] - weights[item2] + weights[item1];
}

int BPPCSolution::computeExcessWeight() const {
    int total = 0;
    for (size_t b = 0; b < bins.size(); ++b) {
        if (bin_loads[b] > C)
            total += bin_loads[b] - C;
    }
    return total;
}

int BPPCSolution::computeConflicts() const {
    int total = 0;
    for (const auto& bin : bins) {
        for (size_t i = 0; i < bin.size(); ++i) {
            for (size_t j = i + 1; j < bin.size(); ++j) {
                if (conflicts[bin[i]].count(bin[j]))
                    total++;
            }
        }
    }
    return total;
}

int BPPCSolution::binsUsed() const {
    int count = 0;
    for (const auto& bin : bins) {
        if (!bin.empty()) count++;
    }
    return count;
}

void BPPCSolution::removeEmptyBins() {
    std::vector<Bin> new_bins;
    std::vector<int> new_loads;

    for (size_t i = 0; i < bins.size(); i++) {
        if (!bins[i].empty()) {
            new_bins.push_back(bins[i]);
            new_loads.push_back(bin_loads[i]);
        }
    }

    bins = new_bins;
    bin_loads = new_loads;
}

int BPPCSolution::getExcess() const {
    return computeExcessWeight();
}

int BPPCSolution::getConflicts() const {
    return computeConflicts();
}

int BPPCSolution::computeObjective(int k1, int k2, int k3) const {
    int used_bins = binsUsed();
    int excess = computeExcessWeight();
    int conflict_count = computeConflicts();
    return k1 * used_bins + k2 * excess + k3 * conflict_count;
}

void BPPCSolution::print() const {
    std::cout << "Solution: \n";
    for (size_t b = 0; b < bins.size(); ++b) {
        if (!bins[b].empty()) {
            std::cout << "Bin " << b << " (load=" << bin_loads[b] << "): ";
            for (int item : bins[b])
                std::cout << item << " ";
            std::cout << "\n";
        }
    }
}

void BPPCSolution::printStatistics(int k1, int k2, int k3) const {
    int obj = computeObjective(k1, k2, k3);

    std::cout << "\nObjective value: " << obj << "\n";
    std::cout << "Bins used: " << binsUsed() << "\n";
    std::cout << "Excess: " << computeExcessWeight() << "\n";
    std::cout << "Conflicts: " << computeConflicts() << "\n";
}
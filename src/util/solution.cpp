#include "solution.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>

BPPCSolution::BPPCSolution(int N_items, int bin_capacity, 
                           const std::vector<int>& item_weights,
                           const std::vector<std::unordered_set<int>>& item_conflicts)
    : N(N_items), C(bin_capacity), weights(item_weights), conflicts(item_conflicts),
      bins_used(0), excess_weight(0), conflicts_count(0)
{
    bins.clear();
    bin_loads.clear();
}

int BPPCSolution::itemConflicts(int item, int bin_index) const {
    if (bin_index >= bins.size()) return 0;
    const auto& bin = bins[bin_index];
    int total = 0;
    for (int other : bin) {
        if (other != item && conflicts[other].count(item))
            total++;
    }
    return total;
}

void BPPCSolution::addItemToBin(int item, int bin_index) {
    if (item < 0 || item >= weights.size()) return;

    if (bin_index >= bins.size()) {
        bins.resize(bin_index + 1);
        bin_loads.resize(bin_index + 1, 0);
    }

    if (bins[bin_index].empty()) bins_used += 1;

    conflicts_count += itemConflicts(item, bin_index);

    int old_excess = std::max(0, bin_loads[bin_index] - C);

    bins[bin_index].push_back(item);
    bin_loads[bin_index] += weights[item];

    int new_excess = std::max(0, bin_loads[bin_index] - C);
    excess_weight += (new_excess - old_excess);

    // sanityCheck();
}

void BPPCSolution::removeItemFromBin(int item, int bin_index) {
    if (item < 0 || item >= weights.size()) return;
    if (bin_index >= bins.size()) return;

    auto& bin = bins[bin_index];

    auto it = std::find(bin.begin(), bin.end(), item);
    if (it == bin.end()) return;

    conflicts_count -= itemConflicts(item, bin_index);

    int old_excess = std::max(0, bin_loads[bin_index] - C);

    bin.erase(it);
    bin_loads[bin_index] -= weights[item];

    if (bin.empty()) bins_used -= 1;

    int new_excess = std::max(0, bin_loads[bin_index] - C);
    excess_weight += (new_excess - old_excess);

    // sanityCheck();
}

void BPPCSolution::moveItem(int item, int from_bin, int to_bin) {
    if (from_bin >= bins.size() || to_bin < 0) return;

    removeItemFromBin(item, from_bin);

    addItemToBin(item, to_bin);
}

void BPPCSolution::swapItems(int bin1, int idx1, int bin2, int idx2) {
    if (bin1 == bin2) return;
    if (bin1 >= bins.size() || bin2 >= bins.size()) return;
    if (idx1 >= bins[bin1].size() || idx2 >= bins[bin2].size()) return;

    int item1 = bins[bin1][idx1];
    int item2 = bins[bin2][idx2];

    removeItemFromBin(item1, bin1);
    removeItemFromBin(item2, bin2);

    addItemToBin(item1, bin2);
    addItemToBin(item2, bin1);
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

bool BPPCSolution::isFeasible() const {
    return (excess_weight == 0 && conflicts_count == 0);
}

int BPPCSolution::getExcess() const {
    return computeExcessWeight();
}

int BPPCSolution::getConflicts() const {
    return computeConflicts();
}

int BPPCSolution::computeObjective(int k1, int k2, int k3) const {
    return k1 * bins_used + k2 * excess_weight + k3 * conflicts_count;
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

void BPPCSolution::sanityCheck() const {
    int real_bins_used = binsUsed();
    int real_excess = computeExcessWeight();
    int real_conflicts = computeConflicts();

    std::cout << "Sanity Check:\n";
    std::cout << "  bins_used:       " << bins_used << " (computed: " << real_bins_used << ")\n";
    std::cout << "  excess_weight:   " << excess_weight << " (computed: " << real_excess << ")\n";
    std::cout << "  conflicts_count: " << conflicts_count << " (computed: " << real_conflicts << ")\n";

    assert(bins_used == real_bins_used && "bins_used does not match recomputed value!");
    assert(excess_weight == real_excess && "excess_weight does not match recomputed value!");
    assert(conflicts_count == real_conflicts && "conflicts_count does not match recomputed value!");
}

static int excessDelta(int load_before, int load_after, int C) {
    int old_excess = std::max(0, load_before - C);
    int new_excess = std::max(0, load_after - C);
    return new_excess - old_excess;
}

int BPPCSolution::deltaAdd(int item, int bin_index, int k1, int k2, int k3) const {
    int d_bins = 0;
    int d_excess = 0;

    bool new_bin = (bin_index >= bins.size() || bins[bin_index].empty());

    if (new_bin) d_bins = 1;

    int load_before = (bin_index < bin_loads.size()) ? bin_loads[bin_index] : 0;
    int load_after = load_before + weights[item];

    d_excess = excessDelta(load_before, load_after, C);

    int d_conflicts = itemConflicts(item, bin_index);

    return k1 * d_bins + k2 * d_excess + k3 * d_conflicts;
}

int BPPCSolution::deltaRemove(int item, int bin_index, int k1, int k2, int k3) const {
    int d_bins = 0;
    int d_excess = 0;

    if (bin_index >= bins.size()) return 0;

    bool becomes_empty = (bins[bin_index].size() == 1);
    if (becomes_empty) d_bins = -1;

    int load_before = bin_loads[bin_index];
    int load_after = load_before - weights[item];

    d_excess = excessDelta(load_before, load_after, C);

    int d_conflicts = -itemConflicts(item, bin_index);

    return k1*d_bins + k2*d_excess + k3*d_conflicts;
}

int BPPCSolution::deltaMove(int item, int from_bin, int to_bin, int k1, int k2, int k3) const {
    if (from_bin == to_bin) return 0;

    int delta = 0;

    delta += deltaRemove(item, from_bin, k1, k2, k3);
    delta += deltaAdd(item, to_bin, k1, k2, k3);

    return delta;
}

int BPPCSolution::deltaSwap(int bin1, int idx1, int bin2, int idx2,
                            int k1, int k2, int k3) const {
    if (bin1 == bin2) return 0;

    int item1 = bins[bin1][idx1];
    int item2 = bins[bin2][idx2];

    // --- BIN DELTA ---
    int d_bins = 0;

    // --- EXCESS DELTA ---
    int load1_before = bin_loads[bin1];
    int load2_before = bin_loads[bin2];

    int load1_after = load1_before - weights[item1] + weights[item2];
    int load2_after = load2_before - weights[item2] + weights[item1];

    int d_excess =
        excessDelta(load1_before, load1_after, C) +
        excessDelta(load2_before, load2_after, C);

    // --- CONFLICT DELTA ---
    int d_conflicts = 0;

    for (int other : bins[bin1]) {
        if (other == item1) continue;
        if (conflicts[other].count(item2)) d_conflicts++;
        if (conflicts[other].count(item1)) d_conflicts--;
    }

    for (int other : bins[bin2]) {
        if (other == item2) continue;
        if (conflicts[other].count(item1)) d_conflicts++;
        if (conflicts[other].count(item2)) d_conflicts--;
    }

    return k1*d_bins + k2*d_excess + k3*d_conflicts;
}
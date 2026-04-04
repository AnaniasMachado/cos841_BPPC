#include "builder.hpp"
#include <algorithm>
#include <iostream>
#include <climits>

SolutionBuilder::SolutionBuilder(const BPPCInstance& instance)
    : inst(instance)
{
    std::random_device rd;
    rng = std::mt19937(rd());
}

// -------------------- Modified First Fit Decreasing --------------------
BPPCSolution SolutionBuilder::MFFD() {
    BPPCSolution sol(inst.N, inst.C, inst.weights, inst.conflicts);

    // 0-based indexing
    std::vector<int> items(inst.N);
    for (int i = 0; i < inst.N; i++) items[i] = i;

    // Sort items decreasing by weight
    std::sort(items.begin(), items.end(), [&](int a, int b) {
        return inst.weights[a] > inst.weights[b];
    });

    for (int item : items) {
        bool placed = false;

        for (size_t bin_idx = 0; bin_idx < sol.bins.size(); bin_idx++) {
            bool conflict = false;

            for (int other : sol.bins[bin_idx]) {
                if (inst.conflicts[item].count(other)) {
                    conflict = true;
                    break;
                }
            }

            if (!conflict &&
                sol.bin_loads[bin_idx] + inst.weights[item] <= inst.C) {

                sol.addItemToBin(item, bin_idx);
                placed = true;
                break;
            }
        }

        if (!placed) {
            sol.addItemToBin(item, sol.bins.size());
        }
    }

    return sol;
}

// -------------------- Random Feasible Solution --------------------
BPPCSolution SolutionBuilder::randomFeasible() {
    BPPCSolution sol(inst.N, inst.C, inst.weights, inst.conflicts);

    std::vector<int> items(inst.N);
    for (int i = 0; i < inst.N; i++) items[i] = i;

    std::shuffle(items.begin(), items.end(), rng);

    for (int item : items) {
        bool placed = false;

        std::vector<size_t> bin_order(sol.bins.size());
        for (size_t i = 0; i < sol.bins.size(); i++) bin_order[i] = i;

        std::shuffle(bin_order.begin(), bin_order.end(), rng);

        for (size_t bin_idx : bin_order) {
            bool conflict = false;

            for (int other : sol.bins[bin_idx]) {
                if (inst.conflicts[item].count(other)) {
                    conflict = true;
                    break;
                }
            }

            if (!conflict &&
                sol.bin_loads[bin_idx] + inst.weights[item] <= inst.C) {

                sol.addItemToBin(item, bin_idx);
                placed = true;
                break;
            }
        }

        if (!placed) {
            sol.addItemToBin(item, sol.bins.size());
        }
    }

    return sol;
}

// -------------------- Greedy Solution (delta version) --------------------
BPPCSolution SolutionBuilder::greedy(double alpha, int k1, int k2, int k3) {
    BPPCSolution sol(inst.N, inst.C, inst.weights, inst.conflicts);

    std::vector<int> items(inst.N);
    for (int i = 0; i < inst.N; i++) items[i] = i;

    std::sort(items.begin(), items.end(), [&](int a, int b) {
        return inst.weights[a] > inst.weights[b];
    });

    for (int item : items) {

        std::vector<std::pair<int,long long>> candidates;
        long long best = LLONG_MAX;
        long long worst = LLONG_MIN;

        long long current_obj = (long long)sol.computeObjective(k1, k2, k3);

        // -------------------- Existing bins --------------------
        for (size_t bin_idx = 0; bin_idx < sol.bins.size(); bin_idx++) {

            int delta = sol.deltaAdd(item, bin_idx, k1, k2, k3);
            long long obj = current_obj + (long long)delta;

            candidates.push_back({(int)bin_idx, obj});

            best = std::min(best, obj);
            worst = std::max(worst, obj);
        }

        // -------------------- New bin --------------------
        {
            size_t new_bin = sol.bins.size();

            int delta = sol.deltaAdd(item, new_bin, k1, k2, k3);
            long long obj = current_obj + (long long)delta;

            candidates.push_back({(int)new_bin, obj});

            best = std::min(best, obj);
            worst = std::max(worst, obj);
        }

        // -------------------- RCL --------------------
        std::vector<int> RCL;
        double threshold = (double)best + alpha * ((double)worst - (double)best);

        for (auto& [bin_idx, obj] : candidates) {
            if ((double)obj <= threshold) {
                RCL.push_back(bin_idx);
            }
        }

        // -------------------- Safety fallback --------------------
        if (RCL.empty()) {
            // pick best deterministically
            int best_bin = candidates[0].first;
            long long best_obj = candidates[0].second;

            for (auto& [bin_idx, obj] : candidates) {
                if (obj < best_obj) {
                    best_obj = obj;
                    best_bin = bin_idx;
                }
            }
            RCL.push_back(best_bin);
        }

        // -------------------- Selection --------------------
        std::uniform_int_distribution<int> dist(0, RCL.size() - 1);
        int chosen_bin = RCL[dist(rng)];

        sol.addItemToBin(item, chosen_bin);
    }

    sol.removeEmptyBins();
    return sol;
}
#include "local_search.hpp"
#include <algorithm>

// -------------------- Constructor --------------------
LocalSearch::LocalSearch(
            BPPCSolution& solution,
            ImprovementType improvement_type_,
            int k1, int k2, int k3)
    : sol(&solution), improvement_type(improvement_type_), K1(k1), K2(k2), K3(k3) {}

// -------------------- Set Solution --------------------
void LocalSearch::setSolution(BPPCSolution& solution) {
    sol = &solution;
}

// -------------------- Relocation (delta version) --------------------
bool LocalSearch::relocation() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;
    int best_from = -1, best_to = -1, best_item = -1;

    // for (size_t from = 0; from < sol->bins.size(); from++) {
    for (int from : sol->bad_bins) {
        for (size_t i = 0; i < sol->bins[from].size(); i++) {
            int item = sol->bins[from][i];

            for (size_t to = 0; to < sol->bins.size(); to++) {
                if (to == from) continue;

                int delta = sol->deltaMove(item, from, to, K1, K2, K3);

                if (improvement_type == ImprovementType::FI && delta < 0) {
                    // First improvement: apply immediately
                    sol->moveItem(item, from, to);
                    sol->removeEmptyBins();
                    return true;
                }

                if (delta < best_delta) {
                    best_delta = delta;
                    best_from = from;
                    best_to = to;
                    best_item = item;
                }
            }
        }
    }

    // Best improvement: apply the best move found
    if (improvement_type == ImprovementType::BI && best_item != -1) {
        sol->moveItem(best_item, best_from, best_to);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Exchange (delta version) --------------------
bool LocalSearch::exchange() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;
    int b1_best = -1, b2_best = -1;
    int i_best = -1, j_best = -1;

    // for (size_t b1 = 0; b1 < sol->bins.size(); b1++) {
    for (int b1 : sol->bad_bins) {
        for (size_t i = 0; i < sol->bins[b1].size(); i++) {

            // for (size_t b2 = b1 + 1; b2 < sol->bins.size(); b2++) {
            for (size_t b2 = 0; b2 < sol->bins.size(); b2++) {
                if ((int)b2 == b1) continue;
                for (size_t j = 0; j < sol->bins[b2].size(); j++) {

                    int delta = sol->deltaSwap(b1, i, b2, j, K1, K2, K3);

                    if (improvement_type == ImprovementType::FI && delta < 0) {
                        // First improvement: apply immediately
                        sol->swapItems(b1, i, b2, j);
                        sol->removeEmptyBins();
                        return true;
                    }

                    if (delta < best_delta) {
                        best_delta = delta;
                        b1_best = b1;
                        b2_best = b2;
                        i_best = i;
                        j_best = j;
                    }
                }
            }
        }
    }

    // Best improvement: apply the best swap found
    if (improvement_type == ImprovementType::BI && i_best != -1) {
        sol->swapItems(b1_best, i_best, b2_best, j_best);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Add (delta version) --------------------
bool LocalSearch::add() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;
    int best_from = -1, best_item = -1;
    size_t new_bin = sol->bins.size();

    // for (size_t b = 0; b < sol->bins.size(); b++) {
    for (int b : sol->bad_bins) {
        for (size_t i = 0; i < sol->bins[b].size(); i++) {
            int item = sol->bins[b][i];

            int delta = sol->deltaMove(item, b, new_bin, K1, K2, K3);

            if (improvement_type == ImprovementType::FI && delta < 0) {
                // First improvement: apply immediately
                sol->moveItem(item, b, new_bin);
                sol->removeEmptyBins();
                return true;
            }

            if (delta < best_delta) {
                best_delta = delta;
                best_from = b;
                best_item = item;
            }
        }
    }

    // Best improvement: apply the best move found
    if (improvement_type == ImprovementType::BI && best_item != -1) {
        sol->moveItem(best_item, best_from, new_bin);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Ejection (delta version) --------------------
bool LocalSearch::ejection() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;
    int best_bin = -1;
    std::vector<int> best_subset;

    size_t new_bin = sol->bins.size();

    // -------------------- Iterate over bins --------------------
    // for (size_t b = 0; b < sol->bins.size(); b++) {
    for (int b : sol->bad_bins) {

        const auto& bin = sol->bins[b];
        if (bin.size() <= 1) continue;

        // -------------------- Sort items by removal cost --------------------
        std::vector<int> items = bin;

        std::sort(items.begin(), items.end(),
                  [&](int a, int b_item) {
                      return sol->deltaRemove(a, b, K1, K2, K3)
                           < sol->deltaRemove(b_item, b, K1, K2, K3);
                  });

        int max_k = std::min(3, (int)items.size());

        // -------------------- Try subsets of size 1..k --------------------
        std::vector<int> subset;

        for (int k = 0; k < max_k; k++) {

            subset.push_back(items[k]);

            int delta =
                sol->deltaRemoveMultiple(subset, b, K1, K2, K3) +
                sol->deltaAddMultiple(subset, new_bin, K1, K2, K3);

            if (improvement_type == ImprovementType::FI && delta < 0) {
                // First improvement: apply immediately
                size_t new_bin_idx = sol->bins.size();
                for (int item : subset) {
                    sol->moveItem(item, b, new_bin_idx);
                }
                sol->removeEmptyBins();
                return true;
            }

            if (delta < best_delta) {
                best_delta = delta;
                best_bin = b;
                best_subset = subset;
            }
        }
    }

    // -------------------- Apply best move --------------------
    if (improvement_type == ImprovementType::BI && best_bin != -1) {
        size_t new_bin_idx = sol->bins.size();

        for (int item : best_subset) {
            sol->moveItem(item, best_bin, new_bin_idx);
        }

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Objective --------------------
int LocalSearch::computeObjective(const BPPCSolution& s) const {
    return s.computeObjective(K1, K2, K3);
}
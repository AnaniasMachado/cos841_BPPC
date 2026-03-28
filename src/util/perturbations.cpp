#include "perturbations.hpp"
#include <algorithm>

Perturbations::Perturbations() {
    std::random_device rd;
    rng = std::mt19937(rd());
}

// -------------------- Relocate k --------------------
void Perturbations::relocateK(BPPCSolution& sol, int k) {
    if (sol.bins.empty()) return;

    std::uniform_int_distribution<size_t> bin_dist(0, sol.bins.size() - 1);

    for (int iter = 0; iter < k; iter++) {
        size_t from = bin_dist(rng);

        if (sol.bins[from].empty()) continue;

        std::uniform_int_distribution<size_t> item_dist(0, sol.bins[from].size() - 1);
        size_t idx = item_dist(rng);
        int item = sol.bins[from][idx];

        size_t to = bin_dist(rng);
        if (to == from) continue;

        sol.moveItem(item, from, to);
    }

    sol.removeEmptyBins();
}

// -------------------- Exchange k --------------------
void Perturbations::exchangeK(BPPCSolution& sol, int k) {
    if (sol.bins.size() < 2) return;

    std::uniform_int_distribution<size_t> bin_dist(0, sol.bins.size() - 1);

    for (int iter = 0; iter < k; iter++) {
        size_t b1 = bin_dist(rng);
        size_t b2 = bin_dist(rng);

        if (b1 == b2) continue;
        if (sol.bins[b1].empty() || sol.bins[b2].empty()) continue;

        std::uniform_int_distribution<size_t> i_dist(0, sol.bins[b1].size() - 1);
        std::uniform_int_distribution<size_t> j_dist(0, sol.bins[b2].size() - 1);

        size_t i = i_dist(rng);
        size_t j = j_dist(rng);

        sol.swapItems(b1, i, b2, j);
    }

    sol.removeEmptyBins();
}

// -------------------- Merge --------------------
void Perturbations::merge(BPPCSolution& sol) {
    if (sol.bins.size() < 2) return;

    std::uniform_int_distribution<size_t> bin_dist(0, sol.bins.size() - 1);

    size_t b1 = bin_dist(rng);
    size_t b2 = bin_dist(rng);

    if (b1 == b2) return;

    // Move all items from b2 to b1
    auto items = sol.bins[b2]; // copy
    for (int item : items) {
        sol.moveItem(item, b2, b1);
    }

    sol.removeEmptyBins();
}

// -------------------- Split --------------------
void Perturbations::split(BPPCSolution& sol) {
    if (sol.bins.empty()) return;

    // Collect bins that can be split (size >= 2)
    std::vector<size_t> candidate_bins;
    for (size_t i = 0; i < sol.bins.size(); i++) {
        if (sol.bins[i].size() > 1) {
            candidate_bins.push_back(i);
        }
    }

    // If no bin can be split, do nothing
    if (candidate_bins.empty()) return;

    // Select a valid bin randomly
    std::uniform_int_distribution<size_t> dist(0, candidate_bins.size() - 1);
    size_t b = candidate_bins[dist(rng)];

    // Copy items (we will modify the original bin)
    std::vector<int> items = sol.bins[b];

    for (int item : items) {
        size_t to;

        if (sol.bins.size() == 1) {
            // Only one bin → must create a new one
            to = sol.bins.size();
        } else {
            // Choose from all bins EXCEPT b, plus the option of a new bin
            size_t max_index = sol.bins.size(); // includes "new bin"
            std::uniform_int_distribution<size_t> to_dist(0, max_index - 1);

            size_t r = to_dist(rng);

            // Shift index to skip b
            if (r >= b) {
                to = r + 1;
            } else {
                to = r;
            }
        }

        sol.moveItem(item, b, to);
    }

    sol.removeEmptyBins();
}
#include "perturbations.hpp"

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

void Perturbations::mergeK(BPPCSolution& sol, int k) {
    int n = sol.bins.size();
    if (n < 2) return;

    // Collect non-empty bins
    std::vector<int> active_bins;
    for (int i = 0; i < n; i++) {
        if (!sol.bins[i].empty()) {
            active_bins.push_back(i);
        }
    }

    if (active_bins.size() < 2) return;

    // Shuffle once
    std::shuffle(active_bins.begin(), active_bins.end(), rng);

    int max_pairs = active_bins.size() / 2;
    int num_merges = std::min(k, max_pairs);

    for (int i = 0; i < num_merges; i++) {
        int b1 = active_bins[2 * i];
        int b2 = active_bins[2 * i + 1];

        if (b1 == b2) continue; // safety (should not happen)

        // Move all items from b2 to b1
        auto items = sol.bins[b2]; // copy
        for (int item : items) {
            sol.moveItem(item, b2, b1);
        }
    }

    // Clean up once at the end
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

void Perturbations::splitK(BPPCSolution& sol, int k) {
    if (sol.bins.empty() || k <= 0) return;

    // -------------------- Collect bins that can be split (size >= 2) --------------------
    std::vector<int> candidates;
    for (size_t i = 0; i < sol.bins.size(); i++) {
        if (sol.bins[i].size() > 1) {
            candidates.push_back(i);
        }
    }

    // Not enough candidates to split
    if ((int)candidates.size() <= k) return;

    // -------------------- Shuffle candidates --------------------
    std::shuffle(candidates.begin(), candidates.end(), rng);

    // -------------------- Prepare uniform distribution over remaining bins --------------------
    std::uniform_int_distribution<size_t> dist(k, candidates.size() - 1);

    // -------------------- Split items --------------------
    for (int i = 0; i < k; i++) {
        int b = candidates[i];
        auto items = sol.bins[b]; // copy items to split

        for (int item : items) {
            size_t to_idx = dist(rng);           // random index in remaining candidates
            int to_bin = candidates[to_idx];    // map to actual bin index
            sol.moveItem(item, b, to_bin);
        }
    }

    sol.removeEmptyBins();
}

// -------------------- Shake --------------------
void Perturbations::shake(BPPCSolution& sol, int k) {
    if (sol.bins.size() <= 1 || k <= 0) {
        return;
    }

    auto rand_int = [&](int l, int r) {
        std::uniform_int_distribution<int> dist(l, r);
        return dist(rng);
    };

    // -------------------- Collect candidate problematic items from bad bins --------------------
    std::vector<int> problematic_pool;
    std::vector<char> used_item(sol.N, 0);

    for (int b : sol.bad_bins) {
        if (b < 0 || b >= (int)sol.bins.size()) continue;

        for (int item : sol.bins[b]) {
            if (!used_item[item]) {
                used_item[item] = 1;
                problematic_pool.push_back(item);
            }
        }
    }

    // -------------------- Select ~50% problematic items --------------------
    int want_problematic = k / 2;
    std::shuffle(problematic_pool.begin(), problematic_pool.end(), rng);

    std::vector<int> selected;
    selected.reserve(k);

    for (int i = 0; i < std::min(want_problematic, (int)problematic_pool.size()); ++i) {
        selected.push_back(problematic_pool[i]);
    }

    // Mark already selected
    std::vector<char> chosen(sol.N, 0);
    for (int item : selected) {
        chosen[item] = 1;
    }

    // -------------------- Fill remaining slots with random items --------------------
    std::vector<int> random_pool;
    random_pool.reserve(sol.N);

    for (int item = 0; item < sol.N; ++item) {
        if (!chosen[item]) {
            random_pool.push_back(item);
        }
    }

    std::shuffle(random_pool.begin(), random_pool.end(), rng);

    int remaining = k - (int)selected.size();
    for (int i = 0; i < std::min(remaining, (int)random_pool.size()); ++i) {
        selected.push_back(random_pool[i]);
    }

    // -------------------- Randomize relocation order --------------------
    std::shuffle(selected.begin(), selected.end(), rng);

    // -------------------- Relocate selected items --------------------
    for (int item : selected) {
        int from_bin = sol.item_bin[item];

        if ((int)sol.bins.size() <= 1) {
            break;
        }

        int to_bin = rand_int(0, (int)sol.bins.size() - 2);
        if (to_bin >= from_bin) {
            ++to_bin;
        }

        sol.moveItem(item, from_bin, to_bin);
    }

    sol.removeEmptyBins();
}
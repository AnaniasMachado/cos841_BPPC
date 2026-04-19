#include "local_search.hpp"

// -------------------- Constructor --------------------
LocalSearch::LocalSearch(BPPCSolution& solution,
                         ImprovementType improvement_type_,
                         int k1, int k2, int k3)
    : sol(&solution),
      improvement_type(improvement_type_),
      K1(k1), K2(k2), K3(k3)
{
    rng.seed(std::random_device{}());

    K = sol->bins_used;
    best_obj = sol->computeObjective(K1, K2, K3);
    // S_pool = std::max(100, (int)(4.5 * K));
    // S_min = std::max(75, (int)(4.0 * K));
    // S_max = std::max(150, (int)(5.0 * K));
    S_pool = std::max(100, (int)(9.0 * K));
    S_min = std::max(75, (int)(7.0 * K));
    S_max = std::max(150, (int)(12.0 * K));
    // S_pool = std::max(100, (int)(12.5 * K));
    // S_min = std::max(75, (int)(10.0 * K));
    // S_max = std::max(150, (int)(15.0 * K));

    // std::cout << "S_pool: " << S_pool << "\n";
    // std::cout << "S_min: " << S_min << "\n";
    // std::cout << "S_max: " << S_max << "\n";

    pool.clear();
}

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

// -------------------- Exchange 21 (delta version) --------------------
bool LocalSearch::exchange21() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;

    int b1_best = -1, b2_best = -1;
    int a_best = -1, b_best = -1, c_best = -1;

    // Iterate over bad bins for b1
    for (int b1 : sol->bad_bins) {

        const auto& bin1 = sol->bins[b1];
        int sz1 = bin1.size();

        if (sz1 < 2) continue;

        // Pick two distinct items from b1
        for (int i1 = 0; i1 < sz1; i1++) {
            for (int i2 = i1 + 1; i2 < sz1; i2++) {

                for (int b2 = 0; b2 < (int)sol->bins.size(); b2++) {

                    if (b2 == b1) continue;

                    const auto& bin2 = sol->bins[b2];
                    if (bin2.empty()) continue;

                    for (int j = 0; j < (int)bin2.size(); j++) {

                        int delta = sol->deltaSwap21(
                            b1, i1, i2,
                            b2, j,
                            K1, K2, K3
                        );

                        // First improvement
                        if (improvement_type == ImprovementType::FI && delta < 0) {

                            int a = sol->bins[b1][i1];
                            int b = sol->bins[b1][i2];
                            int c = sol->bins[b2][j];

                            // apply move safely using item IDs
                            sol->moveItem(a, b1, b2);
                            sol->moveItem(b, b1, b2);
                            sol->moveItem(c, b2, b1);

                            sol->removeEmptyBins();
                            return true;
                        }

                        // Best improvement
                        if (delta < best_delta) {

                            best_delta = delta;

                            b1_best = b1;
                            b2_best = b2;

                            a_best = sol->bins[b1][i1];
                            b_best = sol->bins[b1][i2];
                            c_best = sol->bins[b2][j];
                        }
                    }
                }
            }
        }
    }

    // Apply best move
    if (improvement_type == ImprovementType::BI && b1_best != -1) {

        sol->moveItem(a_best, b1_best, b2_best);
        sol->moveItem(b_best, b1_best, b2_best);
        sol->moveItem(c_best, b2_best, b1_best);

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- One iteration of classic local search only --------------------
bool LocalSearch::classic() {
    if (sol->isFeasible()) {
        return false;
    }

    std::vector<int> order = {0, 1, 2};
    std::shuffle(order.begin(), order.end(), rng);

    bool improved = false;

    for (int op : order) {
        switch (op) {
            case 0: improved |= relocation(); break;
            case 1: improved |= exchange(); break;
            case 2: improved |= exchange21(); break;
        }
    }

    return improved;
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
    size_t new_bin = sol->bins.size();
    std::vector<int> best_subset;

    // -------------------- Iterate over bins --------------------
    // for (size_t b = 0; b < sol->bins.size(); b++) {
    for (int b : sol->bad_bins) {

        const auto& bin = sol->bins[b];
        if (bin.size() <= 1) continue;

        // -------------------- Precompute removal costs --------------------
        std::vector<std::pair<int,int>> items_with_cost;
        items_with_cost.reserve(bin.size());

        for (int item : bin) {
            int cost = sol->deltaRemove(item, b, K1, K2, K3);
            items_with_cost.emplace_back(cost, item);
        }

        int max_k = std::min(3, (int)items_with_cost.size());

        // -------------------- Partial sort by removal cost --------------------
        std::partial_sort(
            items_with_cost.begin(),
            items_with_cost.begin() + max_k,
            items_with_cost.end(),
            [](const std::pair<int,int>& a, const std::pair<int,int>& b) {
                return a.first < b.first; // compare by cost
            }
        );

        // -------------------- Try subsets of size 1..k --------------------
        std::vector<int> subset;
        subset.reserve(max_k);

        for (int k = 0; k < max_k; k++) {

            subset.push_back(items_with_cost[k].second);

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

// -------------------- Ejection (improved delta version) --------------------
bool LocalSearch::ejectionGreedy() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;
    int best_bin = -1;
    size_t new_bin = sol->bins.size();
    std::vector<int> best_subset;

    const int MAX_K = 3;
    const int TOP_K = 5;

    // -------------------- Iterate over bins --------------------
    for (int b : sol->bad_bins) {

        const auto& bin = sol->bins[b];
        if (bin.size() <= 1) continue;

        // -------------------- Compute removal costs --------------------
        std::vector<std::pair<int,int>> items_with_cost;
        items_with_cost.reserve(bin.size());

        for (int item : bin) {
            int cost = sol->deltaRemove(item, b, K1, K2, K3);
            items_with_cost.emplace_back(cost, item);
        }

        // -------------------- Select TOP-K cheapest items --------------------
        int limit = std::min(TOP_K, (int)items_with_cost.size());

        std::partial_sort(
            items_with_cost.begin(),
            items_with_cost.begin() + limit,
            items_with_cost.end(),
            [](const std::pair<int,int>& a, const std::pair<int,int>& b) {
                return a.first < b.first;
            }
        );

        // -------------------- First improvement (FI) --------------------
        if (improvement_type == ImprovementType::FI) {

            std::vector<int> subset;
            std::vector<int> froms;
            subset.reserve(MAX_K);
            froms.reserve(MAX_K);

            int delta = 0;

            for (int i = 0; i < limit && (int)subset.size() < MAX_K; i++) {

                int item = items_with_cost[i].second;

                subset.push_back(item);
                froms.push_back(b);

                delta = sol->deltaRemoveMultiple(subset, b, K1, K2, K3)
                      + sol->deltaAddMultiple(subset, new_bin, K1, K2, K3);

                if (delta < 0) {

                    for (int x : subset) {
                        sol->moveItem(x, b, new_bin);
                    }

                    sol->removeEmptyBins();
                    return true;
                }
            }
        }

        // -------------------- Best improvement (BI) --------------------
        else {

            std::vector<int> subset;
            std::vector<int> froms;
            subset.reserve(MAX_K);
            froms.reserve(MAX_K);

            int delta = 0;

            for (int i = 0; i < limit && (int)subset.size() < MAX_K; i++) {

                int item = items_with_cost[i].second;

                subset.push_back(item);
                froms.push_back(b);

                delta = sol->deltaRemoveMultiple(subset, b, K1, K2, K3)
                      + sol->deltaAddMultiple(subset, new_bin, K1, K2, K3);

                if (delta < best_delta) {
                    best_delta = delta;
                    best_bin = b;
                    best_subset = subset;
                }
            }
        }
    }

    // -------------------- Apply best move --------------------
    if (improvement_type == ImprovementType::BI &&
        best_bin != -1 &&
        best_delta < 0) {

        for (int x : best_subset) {
            sol->moveItem(x, best_bin, new_bin);
        }

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

bool LocalSearch::ejectionGC() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;
    int best_bin = -1;
    size_t new_bin = sol->bins.size();
    std::vector<int> best_subset;

    const int TOP_K = 10;     // small but richer pool
    const int MAX_K = 3;     // keep explosion controlled

    struct Cand {
        int item;
        int from;
        int cost;
    };

    // -------------------- Iterate over bins --------------------
    for (int b : sol->bad_bins) {

        const auto& bin = sol->bins[b];
        if (bin.size() <= 1) continue;

        std::vector<Cand> pool;
        pool.reserve(bin.size());

        // -------------------- Build candidate pool --------------------
        for (int item : bin) {
            int cost = sol->deltaRemove(item, b, K1, K2, K3);
            pool.push_back({item, b, cost});
        }

        if (pool.empty()) continue;

        // -------------------- Select TOP-K candidates --------------------
        int limit = std::min(TOP_K, (int)pool.size());

        std::partial_sort(pool.begin(),
                           pool.begin() + limit,
                           pool.end(),
            [](const Cand& a, const Cand& b) {
                return a.cost < b.cost;
            });

        pool.resize(limit);

        // -------------------- FI --------------------
        if (improvement_type == ImprovementType::FI) {

            int n = pool.size();

            for (int mask = 1; mask < (1 << n); mask++) {

                if (__builtin_popcount(mask) > MAX_K) continue;

                std::vector<int> subset;
                std::vector<int> froms;

                for (int i = 0; i < n; i++) {
                    if (mask & (1 << i)) {
                        subset.push_back(pool[i].item);
                        froms.push_back(pool[i].from);
                    }
                }

                int delta =
                    sol->deltaRemoveMultiple(subset, b, K1, K2, K3) +
                    sol->deltaAddMultiple(subset, new_bin, K1, K2, K3);

                if (delta < 0) {

                    for (int x : subset) {
                        sol->moveItem(x, b, new_bin);
                    }

                    sol->removeEmptyBins();
                    return true;
                }
            }
        }

        // -------------------- BI --------------------
        else {

            int n = pool.size();

            for (int mask = 1; mask < (1 << n); mask++) {

                if (__builtin_popcount(mask) > MAX_K) continue;

                std::vector<int> subset;
                std::vector<int> froms;

                for (int i = 0; i < n; i++) {
                    if (mask & (1 << i)) {
                        subset.push_back(pool[i].item);
                        froms.push_back(pool[i].from);
                    }
                }

                int delta =
                    sol->deltaRemoveMultiple(subset, b, K1, K2, K3) +
                    sol->deltaAddMultiple(subset, new_bin, K1, K2, K3);

                if (delta < best_delta) {
                    best_delta = delta;
                    best_bin = b;
                    best_subset = subset;
                }
            }
        }
    }

    // -------------------- Apply best move --------------------
    if (improvement_type == ImprovementType::BI &&
        best_bin != -1 &&
        best_delta < 0) {

        for (int x : best_subset) {
            sol->moveItem(x, best_bin, new_bin);
        }

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Ejection (global candidate version) --------------------
bool LocalSearch::ejectionGlobal() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;
    size_t new_bin = sol->bins.size();

    std::vector<int> best_subset;
    std::vector<int> best_froms;

    const int TOP_K = 10;
    const int MAX_K = 3;

    struct Candidate {
        int item;
        int bin;
        int cost;
    };

    std::vector<Candidate> pool;
    pool.reserve(64);

    // -------------------- Collect candidates --------------------
    for (int b : sol->bad_bins) {

        const auto& bin = sol->bins[b];
        if (bin.size() <= 1) continue;

        for (int item : bin) {
            pool.push_back({
                item,
                b,
                sol->deltaRemove(item, b, K1, K2, K3)
            });
        }
    }

    if (pool.empty()) {
        return false;
    }

    // -------------------- Keep TOP-K --------------------
    int limit = std::min(TOP_K, (int)pool.size());

    std::nth_element(
        pool.begin(),
        pool.begin() + limit,
        pool.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.cost < b.cost;
        }
    );

    pool.resize(limit);

    const int n = (int)pool.size();
    const int max_mask = 1 << n;

    int subset_items[MAX_K];
    int subset_froms[MAX_K];

    // -------------------- Enumerate subsets --------------------
    for (int mask = 1; mask < max_mask; mask++) {

        if (__builtin_popcount(mask) > MAX_K)
            continue;

        int sz = 0;

        // -------------------- Build subset --------------------
        for (int i = 0; i < n; i++) {
            if (mask & (1 << i)) {
                subset_items[sz] = pool[i].item;
                subset_froms[sz] = pool[i].bin;
                sz++;
            }
        }

        // -------------------- Group by bin --------------------
        int delta = 0;

        // since MAX_K is small, brute grouping is fine
        bool used[MAX_K] = {false};

        for (int i = 0; i < sz; i++) {

            if (used[i]) continue;

            int bin = subset_froms[i];

            std::vector<int> group;
            group.push_back(subset_items[i]);
            used[i] = true;

            for (int j = i + 1; j < sz; j++) {
                if (!used[j] && subset_froms[j] == bin) {
                    group.push_back(subset_items[j]);
                    used[j] = true;
                }
            }

            // correct grouped removal
            if (group.size() == 1) {
                delta += sol->deltaRemove(group[0], bin, K1, K2, K3);
            } else {
                delta += sol->deltaRemoveMultiple(group, bin, K1, K2, K3);
            }
        }

        // -------------------- Add to new bin --------------------
        std::vector<int> subset(subset_items, subset_items + sz);

        delta += sol->deltaAddMultiple(subset, new_bin, K1, K2, K3);

        // -------------------- FI --------------------
        if (improvement_type == ImprovementType::FI) {

            if (delta < 0) {

                for (int i = 0; i < sz; i++) {
                    sol->moveItem(subset_items[i], subset_froms[i], new_bin);
                }

                sol->removeEmptyBins();
                return true;
            }
        }

        // -------------------- BI --------------------
        else {

            if (delta < best_delta) {
                best_delta = delta;

                best_subset.assign(subset_items, subset_items + sz);
                best_froms.assign(subset_froms, subset_froms + sz);
            }
        }
    }

    // -------------------- Apply best move --------------------
    if (improvement_type == ImprovementType::BI &&
        best_delta < 0) {

        for (size_t i = 0; i < best_subset.size(); i++) {
            sol->moveItem(best_subset[i], best_froms[i], new_bin);
        }

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Hungarian (min-cost assignment) --------------------
int LocalSearch::hungarian(const std::vector<std::vector<int>>& cost,
                        std::vector<int>& assignment) {
    int n = cost.size();

    std::vector<int> u(n+1), v(n+1), p(n+1), way(n+1);

    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        std::vector<int> minv(n+1, INT_MAX);
        std::vector<char> used(n+1, false);

        do {
            used[j0] = true;
            int i0 = p[j0], delta = INT_MAX, j1 = 0;

            for (int j = 1; j <= n; j++) {
                if (used[j]) continue;

                int cur = cost[i0-1][j-1] - u[i0] - v[j];

                if (cur < minv[j]) {
                    minv[j] = cur;
                    way[j] = j0;
                }

                if (minv[j] < delta) {
                    delta = minv[j];
                    j1 = j;
                }
            }

            for (int j = 0; j <= n; j++) {
                if (used[j]) {
                    u[p[j]] += delta;
                    v[j] -= delta;
                } else {
                    minv[j] -= delta;
                }
            }

            j0 = j1;

        } while (p[j0] != 0);

        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }

    assignment.assign(n, -1);

    for (int j = 1; j <= n; j++) {
        assignment[p[j]-1] = j-1;
    }

    return -v[0]; // total cost
}

// -------------------- Assignment (delta version) --------------------
bool LocalSearch::assignment(int N_ASSIGN) {
    if (sol->bad_bins.empty()) {
        return false;
    }

    const int EPS = 1;
    N_ASSIGN = std::min((int)sol->bins.size() - 1, N_ASSIGN);

    auto rand_int = [&](int l, int r) {
        std::uniform_int_distribution<int> dist(l, r);
        return dist(rng);
    };

    // -------------------- Pivot --------------------
    int idx = rand_int(0, (int)sol->bad_bins.size() - 1);
    auto it = sol->bad_bins.begin();
    std::advance(it, idx);
    int pivot_bin = *it;

    const auto& pivot = sol->bins[pivot_bin];
    if (pivot.empty()) return false;

    int pivot_item = pivot[rand_int(0, (int)pivot.size() - 1)];

    // -------------------- Build item set --------------------
    std::vector<int> items;
    std::vector<int> froms;

    items.push_back(pivot_item);
    froms.push_back(pivot_bin);

    std::vector<char> used(sol->bins.size(), 0);
    used[pivot_bin] = 1;

    std::vector<int> bin_order(sol->bins.size());
    for (int i = 0; i < (int)sol->bins.size(); i++) {
        bin_order[i] = i;
    }

    std::shuffle(bin_order.begin(), bin_order.end(), rng);

    for (int b : bin_order) {

        if ((int)items.size() >= N_ASSIGN + 1)
            break;

        if (used[b]) continue;

        const auto& bin = sol->bins[b];
        if (bin.empty()) continue;

        int item = bin[rand_int(0, (int)bin.size() - 1)];

        items.push_back(item);
        froms.push_back(b);

        used[b] = 1;
    }

    int n = (int)items.size();
    if (n <= 1) return false;

    // -------------------- Build temp solution (remove all) --------------------
    BPPCSolution temp = *sol;

    for (int i = 0; i < n; i++) {
        temp.removeItemFromBin(items[i], froms[i]);
    }

    // -------------------- Cost matrix (holes model) --------------------
    std::vector<std::vector<int>> cost(n, std::vector<int>(n));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {

            cost[i][j] = temp.deltaAdd(
                items[i],
                froms[j],
                K1, K2, K3
            );

            if (items[i] == pivot_item && i == j) {
                cost[i][j] += EPS;
            }
        }
    }

    // -------------------- Hungarian --------------------
    std::vector<int> assign;
    hungarian(cost, assign);

    // -------------------- Reject identity --------------------
    bool identity = true;
    for (int i = 0; i < n; i++) {
        if (assign[i] != i) {
            identity = false;
            break;
        }
    }
    if (identity) return false;

    // -------------------- Build new solution --------------------
    BPPCSolution candidate = temp;

    for (int i = 0; i < n; i++) {
        int item = items[i];
        int to   = froms[assign[i]];

        candidate.addItemToBin(item, to);
    }

    candidate.removeEmptyBins();

    // -------------------- Compare objective --------------------
    int obj_before = sol->computeObjective(K1, K2, K3);
    int obj_after  = candidate.computeObjective(K1, K2, K3);

    // std::cout << "obj_before: " << obj_before << "\n";
    // std::cout << "obj_after : " << obj_after  << "\n";

    // -------------------- Accept if improving --------------------
    if (obj_after < obj_before) {
        *sol = candidate;
        return true;
    }

    return false;
}

bool LocalSearch::repackingGreedy(int N_ATTEMPTS) {
    if (sol->bins.size() <= 1)
        return false;

    auto rand_int = [&](int l, int r) {
        std::uniform_int_distribution<int> dist(l, r);
        return dist(rng);
    };

    int obj_before = sol->computeObjective(K1, K2, K3);

    for (int attempt = 0; attempt < N_ATTEMPTS; attempt++) {

        int pivot_bin = rand_int(0, (int)sol->bins.size() - 1);
        const auto& pivot = sol->bins[pivot_bin];

        if (pivot.size() <= 1)
            continue;

        std::vector<int> items = pivot;

        std::shuffle(items.begin(), items.end(), rng);

        BPPCSolution candidate = *sol;

        // remove all items from pivot bin
        for (int item : items)
            candidate.removeItemFromBin(item, pivot_bin);

        // -------------------- greedy reconstruction --------------------
        for (int item : items) {

            int best_bin = 0;
            int best_cost = INT_MAX;

            for (int b = 0; b < (int)sol->bins.size(); b++) {

                int c = candidate.deltaAdd(item, b, K1, K2, K3);

                if (c < best_cost) {
                    best_cost = c;
                    best_bin = b;
                }
            }

            candidate.addItemToBin(item, best_bin);
        }

        candidate.removeEmptyBins();

        int obj_after = candidate.computeObjective(K1, K2, K3);

        if (obj_after < obj_before) {
            *sol = candidate;
            return true;
        }
    }

    return false;
}

bool LocalSearch::dualPhaseMove(int N_ASSIGN, int N_ATTEMPTS) {

    // BRANCH 1: INFEASIBLE CASE (ASSIGNMENT)
    if (!sol->isFeasible()) {

        return assignment(N_ASSIGN);
    }

    // BRANCH 2: FEASIBLE CASE (RANDOM BIN REPACKING)

    return repackingGreedy(N_ATTEMPTS);
}

// -------------------- Set Covering --------------------
// bool LocalSearch::setCovering() {
//     bool verbose = true;

//     auto log = [&](const std::string& msg) {
//         if (verbose) std::cout << msg << "\n";
//     };

//     const int TIME_LIMIT_MS = 60000;

//     log("\n[SC] START setCovering()");

//     std::shuffle(pool.begin(), pool.end(), rng);

//     if (pool.empty()) {
//         log("[SC] FAILED: empty pool");
//         return false;
//     }

//     const int n_items = sol->N;
//     const int n_cols  = (int)pool.size();
//     int K_upper = K;

//     // ---------------- ADAPTIVE K ----------------
//     if (sol->isFeasible() && K_upper > 1) {
//         K_upper -= 1;
//         log("[SC] MODE: compression (K-1)");
//     } else {
//         log("[SC] MODE: repair (K)");
//     }

//     log("[SC] n_items=" + std::to_string(n_items) +
//         " n_cols=" + std::to_string(n_cols) +
//         " K-1=" + std::to_string(K_upper));

//     std::vector<bool> covered(n_items, false);

//     for (int j = 0; j < n_cols; j++) {
//         for (int i = 0; i < n_items; i++) {
//             if (pool[j].col.mask[i]) {
//                 covered[i] = true;
//             }
//         }
//     }

//     for (int i = 0; i < n_items; i++) {
//         if (!covered[i]) {
//             std::cout << "[SC ERROR] item " << i << " not coverable!\n";
//         }
//     }

//     Highs highs;
//     HighsLp lp;

//     const int n_rows = n_items + 1;

//     lp.num_col_ = n_cols;
//     lp.num_row_ = n_rows;

//     // ---------------- VARIABLES ----------------
//     lp.col_cost_.assign(n_cols, 0.0);
//     lp.col_lower_.assign(n_cols, 0.0);
//     lp.col_upper_.assign(n_cols, 1.0);
//     lp.integrality_.assign(n_cols, HighsVarType::kInteger);

//     for (int j = 0; j < n_cols; j++) {
//         lp.col_cost_[j] = pool[j].col.cost;
//     }

//     // ---------------- ROW BOUNDS ----------------
//     lp.row_lower_.assign(n_rows, 1.0);
//     lp.row_upper_.assign(n_rows, kHighsInf);

//     lp.row_lower_[n_items] = -kHighsInf;
//     lp.row_upper_[n_items] = K_upper;

//     // ---------------- MATRIX ----------------
//     lp.a_matrix_.start_.assign(n_cols + 1, 0);
//     lp.a_matrix_.index_.clear();
//     lp.a_matrix_.value_.clear();

//     int nnz = 0;

//     for (int j = 0; j < n_cols; j++) {

//         lp.a_matrix_.start_[j] = nnz;

//         const auto& col = pool[j].col;

//         for (int i = 0; i < n_items; i++) {
//             if (col.mask[i]) {
//                 lp.a_matrix_.index_.push_back(i);
//                 lp.a_matrix_.value_.push_back(1.0);
//                 nnz++;
//             }
//         }

//         // K constraint row
//         lp.a_matrix_.index_.push_back(n_items);
//         lp.a_matrix_.value_.push_back(1.0);
//         nnz++;
//     }

//     lp.a_matrix_.start_[n_cols] = nnz;

//     log("[SC] nnz=" + std::to_string(nnz));

//     highs.passModel(lp);
//     highs.setOptionValue("output_flag", false);
//     highs.setOptionValue("log_to_console", false);
//     highs.setOptionValue("time_limit", TIME_LIMIT_MS / 1000.0);

//     log("[SC] calling HiGHS...");

//     const HighsStatus status = highs.run();

//     if (status == HighsStatus::kWarning) {
//         log("[SC] WARNING status (possibly time limit)");
//     }

//     if (status != HighsStatus::kOk) {
//         log("[SC] solver error");
//         return false;
//     }

//     const HighsModelStatus model_status = highs.getModelStatus();

//     log("[SC] model status=" + std::to_string((int)model_status));

//     switch (model_status) {
//         case HighsModelStatus::kInfeasible:
//             log("[SC] infeasible restricted pool");
//             return false;

//         case HighsModelStatus::kUnbounded:
//             log("[SC] ERROR: unbounded model");
//             return false;

//         case HighsModelStatus::kModelEmpty:
//             log("[SC] ERROR: empty model");
//             return false;

//         case HighsModelStatus::kLoadError:
//             log("[SC] ERROR: load error");
//             return false;

//         case HighsModelStatus::kOptimal:
//             log("[SC] optimal solution found");
//             break;

//         case HighsModelStatus::kTimeLimit: {
//             log("[SC] time limit reached (non-optimal solution)");

//             double gap = highs.getInfo().mip_gap;
//             log("[SC] MIP gap = " + std::to_string(gap));
//             break;
//         }

//         default:
//             log("[SC] other status: " + std::to_string((int)model_status));
//             break;
//     }

//     if (!highs.getSolution().value_valid) {
//         log("[SC] No feasible solution found within time.");
//         updatePoolSize(false);
//         return false;
//     }

//     const auto& soln = highs.getSolution();

//     std::vector<std::vector<int>> new_bins;
//     new_bins.reserve(K);

//     for (int j = 0; j < n_cols; j++) {
//         if (soln.col_value[j] > 0.5) {
//             new_bins.push_back(pool[j].col.items);
//         }
//     }

//     if (new_bins.empty()) {
//         log("[SC] no solution found");
//         return false;
//     }

//     std::vector<std::vector<int>> repaired_bins = repairSolution(new_bins);

//     size_t new_hash = hash_solution(repaired_bins);
//     size_t old_hash = hash_solution(sol->bins);

//     // ---------------- TABU CHECK ----------------
//     if (new_hash == old_hash) {
//         log("[SC] identical solution. Rejecting.");
//         updatePoolSize(false);
//         return false;
//     }

//     if (isTabu(new_hash)) {
//         log("[SC] TABU solution detected. Rejecting.");
//         updatePoolSize(false);
//         return false;
//     }

//     // ---------------- ACCEPT ----------------
//     int old_obj = sol->computeObjective(K1, K2, K3);

//     BPPCSolution temp_sol = *sol;
//     temp_sol.rebuildSolutionFromBins(repaired_bins);
//     temp_sol.removeEmptyBins();

//     int new_obj = temp_sol.computeObjective(K1, K2, K3);

//     log("[SC] old_obj= " + std::to_string(old_obj));
//     log("[SC] new_obj= " + std::to_string(new_obj));

//     // Only accept improving solutions
//     if (new_obj >= old_obj) {
//         log("[SC] didn't improve objective value solution. Rejecting.");
//         updatePoolSize(false);
//         return false;
//     }
//     log("[SC] improved objective value solution. Accepting.");

//     // ---------------- APPLY ACCEPTED SOLUTION ----------------
//     *sol = temp_sol;

//     updatePoolSize(true);
//     addTabu(new_hash);

//     last_solution_hash = new_hash;

//     log("[SC] SUCCESS");

//     return true;
// }

bool LocalSearch::setCovering() {
    bool verbose = false;

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    const int TIME_LIMIT_MS = 60000;

    log("\n[SC] START setCovering()");

    std::shuffle(pool.begin(), pool.end(), rng);

    if (pool.empty()) {
        log("[SC] FAILED: empty pool");
        return false;
    }

    const int n_items = sol->N;
    const int n_cols  = (int)pool.size();
    int K_upper = K;

    // ---------------- ADAPTIVE K ----------------
    if (sol->isFeasible() && K_upper > 1) {
        K_upper -= 1;
        log("[SC] MODE: compression (K-1)");
    } else {
        log("[SC] MODE: repair (K)");
    }

    log("[SC] n_items=" + std::to_string(n_items) +
        " n_cols=" + std::to_string(n_cols) +
        " K-1=" + std::to_string(K_upper));

    // ---------------- Gurobi model ----------------
    try {

        GRBEnv env = GRBEnv(true);
        env.set("OutputFlag", "0");
        env.start();

        GRBModel model = GRBModel(env);

        model.set(GRB_DoubleParam_TimeLimit, TIME_LIMIT_MS / 1000.0);
        // model.set(GRB_IntParam_Threads, 1);
        model.set(GRB_IntParam_MIPFocus, 1);

        // ---------------- VARIABLES ----------------
        std::vector<GRBVar> x(n_cols);

        for (int j = 0; j < n_cols; j++) {
            x[j] = model.addVar(
                0.0, 1.0,
                pool[j].col.cost,
                GRB_BINARY
            );
        }

        // ---------------- ROW BOUNDS ----------------
        std::vector<GRBLinExpr> cover_constr(n_items);
        GRBLinExpr k_constr = 0;

        for (int i = 0; i < n_items; i++) {
            cover_constr[i] = 0;
        }

        for (int j = 0; j < n_cols; j++) {

            const auto& col = pool[j].col;

            for (int i = 0; i < n_items; i++) {
                if (col.mask[i]) {
                    cover_constr[i] += x[j];
                }
            }

            k_constr += x[j];
        }

        for (int i = 0; i < n_items; i++) {
            model.addConstr(cover_constr[i] >= 1);
        }

        model.addConstr(k_constr <= K_upper);

        log("[SC] calling Gurobi...");

        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_TIME_LIMIT) {
            log("[SC] time limit reached (non-optimal solution)");
        }

        if (status == GRB_INFEASIBLE) {
            log("[SC] infeasible restricted pool");
            return false;
        }

        if (model.get(GRB_IntAttr_SolCount) == 0) {
            log("[SC] no feasible solution found");
            return false;
        }

        if (status != GRB_OPTIMAL && status != GRB_TIME_LIMIT) {
            log("[SC] solver error");
            return false;
        }

        // ---------------- SOLUTION ----------------
        std::vector<std::vector<int>> new_bins;
        new_bins.reserve(K);

        for (int j = 0; j < n_cols; j++) {
            if (x[j].get(GRB_DoubleAttr_X) > 0.5) {
                new_bins.push_back(pool[j].col.items);
            }
        }

        if (new_bins.empty()) {
            log("[SC] no solution found");
            return false;
        }

        std::vector<std::vector<int>> repaired_bins = repairSolution(new_bins);

        size_t new_hash = hash_solution(repaired_bins);
        size_t old_hash = hash_solution(sol->bins);

        // ---------------- TABU CHECK ----------------
        if (new_hash == old_hash) {
            log("[SC] identical solution. Rejecting.");
            updatePoolSize(false);
            return false;
        }

        if (isTabu(new_hash)) {
            log("[SC] TABU solution detected. Rejecting.");
            updatePoolSize(false);
            return false;
        }

        // ---------------- ACCEPT ----------------
        int old_obj = sol->computeObjective(K1, K2, K3);

        BPPCSolution temp_sol = *sol;
        temp_sol.rebuildSolutionFromBins(repaired_bins);
        temp_sol.removeEmptyBins();

        int new_obj = temp_sol.computeObjective(K1, K2, K3);

        log("[SC] old_obj= " + std::to_string(old_obj));
        log("[SC] new_obj= " + std::to_string(new_obj));

        // Only accept improving solutions
        if (new_obj >= old_obj) {
            log("[SC] didn't improve objective value solution. Rejecting.");
            updatePoolSize(false);
            return false;
        }
        log("[SC] improved objective value solution. Accepting.");

        // ---------------- APPLY ACCEPTED SOLUTION ----------------
        *sol = temp_sol;

        updatePoolSize(true);
        addTabu(new_hash);

        last_solution_hash = new_hash;

        log("[SC] SUCCESS");

        return true;

    } catch (GRBException &e) {
        log("[SC] Gurobi error: " + std::string(e.getMessage()));
        return false;
    } catch (...) {
        log("[SC] Unknown Gurobi error");
        return false;
    }
}

void LocalSearch::updateK() {
    if (K > sol->bins_used && sol->isFeasible()) K = sol->bins_used;
}

void LocalSearch::updateElite(const BPPCSolution& candidate) {

    if (candidate.computeObjective(K1, K2, K3) >= best_obj) return;

    sol->bins = candidate.bins;

    elite_bins = candidate.bins;

    elite_hashes.clear();
    for (const auto& bin : elite_bins) {
        elite_hashes.insert(hash_bin(bin));
    }

    elite_solution_hash = hash_solution(elite_bins);
}

void LocalSearch::updatePool() {

    const int n_items = sol->N;

    for (int b = 0; b < (int)sol->bins.size(); b++) {

        const auto& bin = sol->bins[b];
        if (bin.empty()) continue;

        Column col;
        col.items = bin;
        col.buildMask(n_items);

        int load = sol->bin_loads[b];
        int conflicts = sol->bin_conflicts[b];

        int excess = std::max(0, load - sol->C);

        col.cost = K1 + K2 * excess + K3 * conflicts;

        addColumn(std::move(col), n_items);
    }

    trimPool();
}

void LocalSearch::addToPool(const BPPCSolution& s) {

    const int n_items = s.N;

    // ---- Extract columns from solution s ----
    for (int b = 0; b < (int)s.bins.size(); b++) {

        const auto& bin = s.bins[b];
        if (bin.empty()) continue;

        Column col;
        col.items = bin;
        col.buildMask(n_items);

        int load = s.bin_loads[b];
        int conflicts = s.bin_conflicts[b];

        int excess = std::max(0, load - s.C);

        col.cost = K1 + K2 * excess + K3 * conflicts;

        addColumn(std::move(col), n_items);
    }

    trimPool();
}

double LocalSearch::jaccard(const std::vector<int>& a,
                             const std::vector<int>& b) const
{
    int inter = 0;

    for (int i : a) {
        for (int j : b) {
            if (i == j) {
                inter++;
                break;
            }
        }
    }

    int uni = a.size() + b.size() - inter;

    return (uni == 0) ? 0.0 : (double)inter / uni;
}

void LocalSearch::trimPool() {

    if (pool.size() <= S_pool) return;

    while ((int)pool.size() > S_pool) {

        auto worst_it = pool.end();
        double worst_score = std::numeric_limits<double>::infinity();

        for (auto it = pool.begin(); it != pool.end(); ++it) {

            const auto& col = it->col;
            size_t h = it->hash;

            // Never remove elite bins
            if (elite_hashes.count(h)) {
                continue;
            }

            // Compute best Jaccard vs any elite bin
            double best_j = 0.0;

            for (const auto& ebin : elite_bins) {
                double j = jaccard(col.items, ebin);
                if (j > best_j) best_j = j;
            }

            // Small aging tie-break (optional)
            double score = best_j - std::distance(pool.begin(), it) * 1e-9;

            // Remove worst columns
            if (score < worst_score) {
                worst_score = score;
                worst_it = it;
            }
        }

        if (worst_it == pool.end()) break;

        seen.erase(worst_it->hash);
        pool.erase(worst_it);
    }
}

// -------------------- Objective --------------------
int LocalSearch::computeObjective(const BPPCSolution& s) const {
    return s.computeObjective(K1, K2, K3);
}

inline size_t LocalSearch::hash_bin(const std::vector<int>& bin) const {
    static constexpr uint64_t FNV = 1469598103934665603ULL;
    static constexpr uint64_t PRIME = 1099511628211ULL;

    std::vector<int> tmp = bin;
    std::sort(tmp.begin(), tmp.end());

    size_t h = FNV;

    for (int x : tmp)
    {
        h ^= (uint64_t)x + 0x9e3779b97f4a7c15ULL;
        h *= PRIME;
    }

    // final avalanche step
    h ^= (h >> 33);
    h *= 0xff51afd7ed558ccdULL;
    h ^= (h >> 33);
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= (h >> 33);

    return h;
}

inline size_t LocalSearch::hash_solution(const std::vector<std::vector<int>>& bins) const {
    static constexpr uint64_t FNV = 1469598103934665603ULL;
    static constexpr uint64_t PRIME = 1099511628211ULL;

    std::vector<size_t> bin_hashes;
    bin_hashes.reserve(bins.size());

    for (const auto& bin : bins)
    {
        bin_hashes.push_back(hash_bin(bin));
    }

    // canonical order
    std::sort(bin_hashes.begin(), bin_hashes.end());

    size_t h = FNV;

    for (size_t hb : bin_hashes)
    {
        h ^= hb;
        h *= PRIME;
    }

    // final avalanche
    h ^= (h >> 33);
    h *= 0xff51afd7ed558ccdULL;
    h ^= (h >> 33);
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= (h >> 33);

    return h;
}

size_t LocalSearch::hash_pool() const {
    size_t h = 1469598103934665603ULL;

    for (const auto& e : pool) {
        for (int x : e.col.items) {
            h ^= (size_t)x + 0x9e3779b97f4a7c15ULL;
            h *= 1099511628211ULL;
        }

        h ^= (size_t)e.col.cost;
        h *= 1099511628211ULL;
    }

    return h;
}

inline void LocalSearch::addColumn(Column&& col, int n_items)
{
    if ((int)col.mask.size() != n_items) {
        col.buildMask(n_items);
    }

    size_t h = hash_bin(col.items);

    if (seen.find(h) != seen.end())
        return;

    seen.insert(h);

    pool.emplace_back(PoolEntry{
        .col = std::move(col),
        .hash = h
    });
}

bool LocalSearch::isTabu(size_t h) {
    return tabu_solutions.find(h) != tabu_solutions.end();
}

void LocalSearch::addTabu(size_t h) {
    tabu_solutions.insert(h);
    tabu_queue.push_back(h);

    if ((int)tabu_queue.size() > tabu_tenure) {
        size_t old = tabu_queue.front();
        tabu_queue.pop_front();
        tabu_solutions.erase(old);
    }
}

void LocalSearch::updatePoolSize(bool optimal_solved) {

    if (optimal_solved) {
        S_pool = (int)(S_pool * 1.15);
    } else {
        S_pool = (int)(S_pool * 0.85);
    }

    S_pool = std::max(S_pool, S_min);
    S_pool = std::min(S_pool, S_max);
}

std::vector<std::vector<int>> LocalSearch::repairSolution(
    const std::vector<std::vector<int>>& input_bins) {
    std::vector<std::vector<int>> bins = input_bins;

    const int n_items = sol->N;

    // Build item to bins structure
    std::vector<std::vector<int>> item_bins(n_items);

    for (int b = 0; b < (int)bins.size(); b++) {
        for (int item : bins[b]) {
            item_bins[item].push_back(b);
        }
    }

    // Extract duplicated items only
    std::vector<int> dup_items;
    for (int i = 0; i < n_items; i++) {
        if (item_bins[i].size() > 1) {
            dup_items.push_back(i);
        }
    }

    // Best solution tracking
    std::vector<std::vector<int>> best_bins;
    int best_cost = std::numeric_limits<int>::max();

    // DFS recursion
    std::function<void(int)> dfs = [&](int idx)
    {
        // Leaf: all duplicates resolved
        if (idx == (int)dup_items.size()) {

            int cost = 0;
            for (auto& b : bins)
                if (!b.empty()) cost++;

            if (cost < best_cost) {
                best_cost = cost;
                best_bins = bins;
            }

            return;
        }

        int item = dup_items[idx];
        const auto& cand_bins = item_bins[item];

        // Try each bin as "keep bin"
        for (int keep_bin : cand_bins) {

            // Store removed positions for rollback
            std::vector<std::pair<int, int>> removed;

            // Remove item from all other bins
            for (int b : cand_bins) {
                if (b == keep_bin) continue;

                auto& bin = bins[b];

                auto it = std::find(bin.begin(), bin.end(), item);
                if (it != bin.end()) {
                    bin.erase(it);
                    removed.push_back({b, item});
                }
            }

            dfs(idx + 1);

            // Rollback
            for (auto& [b, val] : removed) {
                bins[b].push_back(val);
            }
        }
    };

    // Run DP/DFS
    dfs(0);

    return best_bins;
}

bool LocalSearch::kempeChain() {
    if (sol->bin_conflicts.empty()) return false;

    bool full_clean = false;
    bool improved_global = false;

    while (true) {

        bool improved = false;

        int n_bins = sol->bins.size();
        if (n_bins < 2) break;

        // -------------------- Copy + shuffle bad bins --------------------
        std::vector<int> bad_list(sol->bad_bins.begin(), sol->bad_bins.end());
        std::shuffle(bad_list.begin(), bad_list.end(), rng);

        for (int a : bad_list) {

            if (sol->bin_conflicts[a] == 0) continue;

            // optional: also shuffle target bins
            std::vector<int> order_b(n_bins);
            std::iota(order_b.begin(), order_b.end(), 0);
            std::shuffle(order_b.begin(), order_b.end(), rng);

            for (int b : order_b) {

                if (a == b) continue;

                const auto& A = sol->bins[a];
                const auto& B = sol->bins[b];

                if (A.empty() || B.empty()) continue;

                // -------------------- Select seed WITH conflicts --------------------
                int seed = -1;

                // also shuffle items for diversification
                std::vector<int> A_shuffled = A;
                std::shuffle(A_shuffled.begin(), A_shuffled.end(), rng);

                for (int x : A_shuffled) {
                    if (sol->itemConflicts(x, a) > 0) {
                        seed = x;
                        break;
                    }
                }

                if (seed == -1) continue;

                // -------------------- Build Kempe Chain --------------------
                std::vector<int> chainA, chainB;
                std::vector<char> inA(sol->N, 0), inB(sol->N, 0);

                chainA.push_back(seed);
                inA[seed] = 1;

                bool changed = true;

                while (changed) {
                    changed = false;

                    // expand into B
                    for (int u : B) {
                        if (inB[u]) continue;

                        for (int v : chainA) {
                            if (sol->hasConflict(u, v)) {
                                chainB.push_back(u);
                                inB[u] = 1;
                                changed = true;
                                break;
                            }
                        }
                    }

                    // expand into A
                    for (int u : A) {
                        if (inA[u]) continue;

                        for (int v : chainB) {
                            if (sol->hasConflict(u, v)) {
                                chainA.push_back(u);
                                inA[u] = 1;
                                changed = true;
                                break;
                            }
                        }
                    }
                }

                if (chainA.empty() && chainB.empty()) continue;

                // -------------------- Evaluate --------------------
                int delta = sol->deltaSwapSubset(a, chainA, b, chainB,
                                                K1, K2, K3);

                if (delta >= 0) continue;

                // -------------------- Apply move --------------------
                for (int x : chainA)
                    sol->moveItem(x, a, b);

                for (int x : chainB)
                    sol->moveItem(x, b, a);

                sol->removeEmptyBins();

                improved = true;
                improved_global = true;

                break; // restart search
            }

            if (improved) break;
        }

        // -------------------- Stop conditions --------------------
        if (!full_clean) break;
        if (!improved) break;
    }

    return improved_global;
}

void LocalSearch::generateColumns() {
    bool verbose = false;

    int N_COLUMNS = 25;
    int PRICING_ITERS = 12;
    int SEEDS_PER_ITER = 10;
    int LOCAL_STEPS = 15;

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    const int N = sol->N;
    size_t before = pool.size();

    std::uniform_int_distribution<int> rnd(0, N - 1);
    std::uniform_real_distribution<double> noise(0.0, 1e-4);

    std::unordered_set<size_t> local_seen;
    std::vector<double> dual_values(N, 0.0);

    // ---------------- LP ONCE ----------------
    try {
        GRBEnv env(true);
        env.set("OutputFlag", "0");
        env.start();

        GRBModel model(env);

        int n_cols = (int)pool.size();
        if (n_cols == 0) return;

        std::vector<GRBVar> x(n_cols);

        for (int j = 0; j < n_cols; j++)
            x[j] = model.addVar(0.0, 1.0, pool[j].col.cost, GRB_CONTINUOUS);

        std::vector<GRBConstr> cover(N);

        for (int i = 0; i < N; i++) {
            GRBLinExpr expr = 0;
            for (int j = 0; j < n_cols; j++)
                if (pool[j].col.mask[i])
                    expr += x[j];

            cover[i] = model.addConstr(expr >= 1);
        }

        model.optimize();

        if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL)
            return;

        for (int i = 0; i < N; i++)
            dual_values[i] = cover[i].get(GRB_DoubleAttr_Pi);

    } catch (...) {
        return;
    }

    // ---------------- COST ----------------
    auto reducedCost = [&](const std::vector<int>& items) {
        double dual_sum = 0.0;
        int load = 0;
        int conflicts = 0;

        for (int i = 0; i < (int)items.size(); i++) {
            int u = items[i];
            dual_sum += dual_values[u];
            load += sol->weights[u];

            for (int j = i + 1; j < (int)items.size(); j++)
                if (sol->hasConflict(u, items[j]))
                    conflicts++;
        }

        int excess = std::max(0, load - sol->C);
        double cost = K1 + K2 * excess + K3 * conflicts;

        return cost - dual_sum;
    };

    auto evaluate = [&](const std::vector<int>& items) {
        return reducedCost(items);
    };

    // ---------------- TRY ADD ----------------
    auto try_add = [&](const std::vector<int>& items) {
        if (items.size() < 2) return false;

        size_t h = hash_bin(items);
        if (local_seen.count(h)) return false;
        local_seen.insert(h);

        Column col;
        col.items = items;
        col.buildMask(N);

        int load = 0;
        int conflicts = 0;

        for (int i = 0; i < (int)items.size(); i++) {
            load += sol->weights[items[i]];
            for (int j = i + 1; j < (int)items.size(); j++)
                if (sol->hasConflict(items[i], items[j]))
                    conflicts++;
        }

        int excess = std::max(0, load - sol->C);
        col.cost = K1 + K2 * excess + K3 * conflicts;

        addColumn(std::move(col), N);
        return true;
    };

    // ---------------- LOCAL SEARCH ----------------
    auto improve = [&](std::vector<int>& items) {

        std::vector<char> used(N, false);
        int load = 0;

        for (int i : items) {
            used[i] = true;
            load += sol->weights[i];
        }

        for (int step = 0; step < LOCAL_STEPS; step++) {

            bool improved = false;

            // --- TRY ADD ---
            for (int t = 0; t < 20; t++) {

                int i = rnd(rng);
                if (used[i]) continue;

                if (load + sol->weights[i] > sol->C)
                    continue;

                const auto& row = sol->conflicts[i];
                bool ok = true;

                for (int j : items) {
                    if (row[j >> 6] & (1ULL << (j & 63))) {
                        ok = false;
                        break;
                    }
                }

                if (!ok) continue;

                double gain = dual_values[i];

                if (gain > 0) {
                    items.push_back(i);
                    used[i] = true;
                    load += sol->weights[i];
                    improved = true;
                    break;
                }
            }

            if (improved) continue;

            // --- TRY REMOVE WORST ---
            int worst_pos = -1;
            double worst_gain = 1e18;

            for (int i = 0; i < (int)items.size(); i++) {
                int u = items[i];
                double g = dual_values[u];

                if (g < worst_gain) {
                    worst_gain = g;
                    worst_pos = i;
                }
            }

            if (worst_pos != -1 && worst_gain < 0) {
                int u = items[worst_pos];
                load -= sol->weights[u];
                used[u] = false;
                items.erase(items.begin() + worst_pos);
                improved = true;
            }

            if (!improved)
                break;
        }
    };

    // ---------------- MAIN LOOP ----------------
    int generated = 0;

    for (int it = 0; it < PRICING_ITERS; it++) {

        for (int seed_try = 0; seed_try < SEEDS_PER_ITER; seed_try++) {

            std::vector<int> items;
            std::vector<char> used(N, false);
            int load = 0;

            // --- greedy seed construction ---
            for (int step = 0; step < 50; step++) {

                int best = -1;
                double best_score = 0;

                for (int i = 0; i < N; i++) {

                    if (used[i]) continue;
                    if (load + sol->weights[i] > sol->C) continue;

                    const auto& row = sol->conflicts[i];
                    bool ok = true;

                    for (int j : items) {
                        if (row[j >> 6] & (1ULL << (j & 63))) {
                            ok = false;
                            break;
                        }
                    }

                    if (!ok) continue;

                    double score = dual_values[i] + noise(rng);

                    if (score > best_score) {
                        best_score = score;
                        best = i;
                    }
                }

                if (best == -1)
                    break;

                items.push_back(best);
                used[best] = true;
                load += sol->weights[best];
            }

            if (items.empty())
                continue;

            // --- LOCAL IMPROVEMENT ---
            improve(items);

            double rc = evaluate(items);

            if (rc < -1e-6) {
                if (try_add(items)) {
                    generated++;
                    if (generated >= N_COLUMNS)
                        break;
                }
            }
        }

        if (generated >= N_COLUMNS)
            break;
    }

    log("[CG] added=" + std::to_string(pool.size() - before));
    log("[CG] pool_after=" + std::to_string(pool.size()));

    trimPool();
}


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

    const int K = (int)solution.bins.size();
    S_pool = std::max(100, (int)(2.5 * K));
    S_min = std::max(75, (int)(2.0 * K));
    S_max = std::max(150, (int)(3.0 * K));

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
int LocalSearch::hungarian(
    const std::vector<std::vector<int>>& cost,
    std::vector<int>& assignment)
{
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
bool LocalSearch::assignment() {

    if (sol->bad_bins.empty()) {
        return false;
    }

    const int N_ASSIGN = 9;
    const int EPS = 1;

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

    items.reserve(N_ASSIGN + 1);
    froms.reserve(N_ASSIGN + 1);

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

    // -------------------- Cost matrix --------------------
    std::vector<std::vector<int>> cost(n, std::vector<int>(n));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {

            int bin_j = froms[j];

            cost[i][j] = sol->deltaAdd(items[i], bin_j, K1, K2, K3);

            if (items[i] == pivot_item && i == j) {
                cost[i][j] += EPS;
            }
        }
    }

    // -------------------- Hungarian --------------------
    std::vector<int> assignment;
    hungarian(cost, assignment);

    // -------------------- Delta --------------------
    int delta = 0;

    for (int i = 0; i < n; i++) {
        delta += sol->deltaRemove(items[i], froms[i], K1, K2, K3);
    }

    for (int i = 0; i < n; i++) {
        delta += sol->deltaAdd(items[i], froms[assignment[i]], K1, K2, K3);
    }

    // -------------------- Apply move --------------------
    if (delta < 0) {

        std::vector<std::tuple<int,int,int>> moves;
        moves.reserve(n);

        for (int i = 0; i < n; i++) {
            moves.emplace_back(items[i], froms[i], froms[assignment[i]]);
        }

        for (auto [item, from, to] : moves) {
            sol->moveItem(item, from, to);
        }

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Set Covering --------------------
bool LocalSearch::setCovering() {
    bool verbose = false;

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    const int TIME_LIMIT_MS = 500;

    log("\n[SC] START setCovering()");

    updatePool();

    if (pool.empty()) {
        log("[SC] FAILED: empty pool");
        return false;
    }

    const int n_items = sol->N;
    const int n_cols  = (int)pool.size();

    // ---------------- ADAPTIVE K ----------------
    int K = (int)sol->bins.size();

    if (sol->isFeasible() && K > 1) {
        K -= 1;
        log("[SC] MODE: compression (K-1)");
    } else {
        log("[SC] MODE: repair (K)");
    }

    log("[SC] n_items=" + std::to_string(n_items) +
        " n_cols=" + std::to_string(n_cols) +
        " K=" + std::to_string(K));

    Highs highs;
    HighsLp lp;

    const int n_rows = n_items + 1;

    lp.num_col_ = n_cols;
    lp.num_row_ = n_rows;

    // ---------------- VARIABLES ----------------
    lp.col_cost_.assign(n_cols, 0.0);
    lp.col_lower_.assign(n_cols, 0.0);
    lp.col_upper_.assign(n_cols, 1.0);
    lp.integrality_.assign(n_cols, HighsVarType::kInteger);

    for (int j = 0; j < n_cols; j++) {
        lp.col_cost_[j] = pool[j].col.cost;
    }

    // const double LAMBDA = 1.0;

    // for (int j = 0; j < n_cols; j++) {
    //     lp.col_cost_[j] = pool[j].col.cost + LAMBDA;
    // }

    // ---------------- ROW BOUNDS ----------------
    lp.row_lower_.assign(n_rows, 1.0);
    lp.row_upper_.assign(n_rows, kHighsInf);

    lp.row_lower_[n_items] = -kHighsInf;
    lp.row_upper_[n_items] = K;

    // ---------------- MATRIX ----------------
    lp.a_matrix_.start_.assign(n_cols + 1, 0);
    lp.a_matrix_.index_.clear();
    lp.a_matrix_.value_.clear();

    int nnz = 0;

    for (int j = 0; j < n_cols; j++) {

        lp.a_matrix_.start_[j] = nnz;

        const auto& col = pool[j].col;

        for (int i = 0; i < n_items; i++) {
            if (col.mask[i]) {
                lp.a_matrix_.index_.push_back(i);
                lp.a_matrix_.value_.push_back(1.0);
                nnz++;
            }
        }

        // K constraint row
        lp.a_matrix_.index_.push_back(n_items);
        lp.a_matrix_.value_.push_back(1.0);
        nnz++;
    }

    lp.a_matrix_.start_[n_cols] = nnz;

    log("[SC] nnz=" + std::to_string(nnz));

    highs.passModel(lp);
    highs.setOptionValue("output_flag", false);
    highs.setOptionValue("log_to_console", false);
    highs.setOptionValue("time_limit", TIME_LIMIT_MS / 1000.0);

    log("[SC] calling HiGHS...");

    const HighsStatus status = highs.run();

    if (status != HighsStatus::kOk) {
        log("[SC] solver error");
        return false;
    }

    const HighsModelStatus model_status = highs.getModelStatus();

    log("[SC] model status=" + std::to_string((int)model_status));

    switch (model_status) {
        case HighsModelStatus::kInfeasible:
            log("[SC] infeasible restricted pool");
            return false;

        case HighsModelStatus::kUnbounded:
            log("[SC] ERROR: unbounded model");
            return false;

        case HighsModelStatus::kModelEmpty:
            log("[SC] ERROR: empty model");
            return false;

        case HighsModelStatus::kLoadError:
            log("[SC] ERROR: load error");
            return false;

        default:
            break;
    }

    const auto& soln = highs.getSolution();

    std::vector<std::vector<int>> new_bins;
    new_bins.reserve(K);

    for (int j = 0; j < n_cols; j++) {
        if (soln.col_value[j] > 0.5) {
            new_bins.push_back(pool[j].col.items);
        }
    }

    if (new_bins.empty()) {
        log("[SC] no solution found");
        return false;
    }

    size_t new_hash = hash_solution(new_bins);
    size_t old_hash = hash_solution(sol->bins);

    // ---------------- TABU CHECK ----------------
    if (isTabu(new_hash)) {
        log("[SC] TABU solution detected. Rejecting.");
        updatePoolSize(false);
        return false;
    }

    if (new_hash == old_hash) {
        log("[SC] identical solution. Rejecting.");
        updatePoolSize(false);
        return false;
    }

    // ---------------- ACCEPT ----------------
    sol->rebuildSolutionFromBins(new_bins);
    sol->removeEmptyBins();

    updatePoolSize(true);
    addTabu(new_hash);

    last_solution_hash = new_hash;

    log("[SC] SUCCESS");

    return true;
}

void LocalSearch::updatePool() {

    const int n_items = sol->N;

    auto hash_bin = [&](const std::vector<int>& bin) -> size_t {
        size_t h = 1469598103934665603ULL;
        for (int x : bin) {
            h ^= (size_t)x + 0x9e3779b97f4a7c15ULL;
            h *= 1099511628211ULL;
        }
        h ^= 0x9e3779b97f4a7c15ULL;
        h *= 1099511628211ULL;
        return h;
    };

    auto add_column = [&](Column&& col) {

        if ((int)col.mask.size() != n_items) {
            col.buildMask(n_items);
        }

        size_t h = hash_bin(col.items);

        if (seen.find(h) != seen.end())
            return;

        seen.insert(h);

        PoolEntry entry;
        entry.col = std::move(col);
        entry.hash = h;

        pool.push_back(std::move(entry));

        while ((int)pool.size() > S_pool) {

            PoolEntry& old = pool.front();

            seen.erase(old.hash);

            pool.pop_front();
        }
    };

    for (int b = 0; b < (int)sol->bins.size(); b++) {

        const auto& bin = sol->bins[b];
        if (bin.empty()) continue;

        Column col;
        col.items = bin;
        col.buildMask(n_items);

        int load = sol->bin_loads[b];
        int conflicts = sol->bin_conflicts[b];

        int excess = std::max(0, load - sol->C);

        col.cost = K2 * excess + K3 * conflicts;

        add_column(std::move(col));
    }
}

// -------------------- Objective --------------------
int LocalSearch::computeObjective(const BPPCSolution& s) const {
    return s.computeObjective(K1, K2, K3);
}

size_t LocalSearch::hash_solution(const std::vector<std::vector<int>>& bins) const {
    size_t h = 1469598103934665603ULL; // FNV-1a

    for (const auto& bin : bins) {
        for (int x : bin) {
            h ^= (size_t)x + 0x9e3779b97f4a7c15ULL;
            h *= 1099511628211ULL;
        }

        // separator to avoid permutation collisions
        h ^= 0x9e3779b97f4a7c15ULL;
        h *= 1099511628211ULL;
    }

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
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
    // S_pool = std::max(100, (int)(6.0 * K));
    // S_min = std::max(75, (int)(5.0 * K));
    // S_max = std::max(150, (int)(7.0 * K));
    // S_pool = std::max(100, (int)(9.0 * K));
    // S_min = std::max(75, (int)(7.0 * K));
    // S_max = std::max(150, (int)(12.0 * K));
    // S_pool = std::max(100, (int)(12.5 * K));
    // S_min = std::max(75, (int)(10.0 * K));
    // S_max = std::max(150, (int)(15.0 * K));

    S_pool = std::max(100, (int)(10.0 * K));

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

// -------------------- Exchange 22 (delta version) --------------------
bool LocalSearch::exchange22() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;

    int b1_best = -1, b2_best = -1;
    int a_best = -1, b_best = -1;
    int c_best = -1, d_best = -1;

    for (int b1 : sol->bad_bins) {
        const auto& bin1 = sol->bins[b1];
        int sz1 = (int)bin1.size();

        if (sz1 < 2) continue;

        for (int i1 = 0; i1 < sz1; ++i1) {
            for (int i2 = i1 + 1; i2 < sz1; ++i2) {

                std::vector<int> subset1 = {
                    sol->bins[b1][i1],
                    sol->bins[b1][i2]
                };

                for (int b2 = 0; b2 < (int)sol->bins.size(); ++b2) {
                    if (b2 == b1) continue;

                    const auto& bin2 = sol->bins[b2];
                    int sz2 = (int)bin2.size();

                    if (sz2 < 2) continue;

                    for (int j1 = 0; j1 < sz2; ++j1) {
                        for (int j2 = j1 + 1; j2 < sz2; ++j2) {

                            std::vector<int> subset2 = {
                                sol->bins[b2][j1],
                                sol->bins[b2][j2]
                            };

                            int delta = sol->deltaSwapSubset(
                                b1, subset1,
                                b2, subset2,
                                K1, K2, K3
                            );

                            if (improvement_type == ImprovementType::FI && delta < 0) {
                                int a = subset1[0];
                                int b = subset1[1];
                                int c = subset2[0];
                                int d = subset2[1];

                                sol->moveItem(a, b1, b2);
                                sol->moveItem(b, b1, b2);
                                sol->moveItem(c, b2, b1);
                                sol->moveItem(d, b2, b1);

                                sol->removeEmptyBins();
                                return true;
                            }

                            if (delta < best_delta) {
                                best_delta = delta;

                                b1_best = b1;
                                b2_best = b2;

                                a_best = subset1[0];
                                b_best = subset1[1];
                                c_best = subset2[0];
                                d_best = subset2[1];
                            }
                        }
                    }
                }
            }
        }
    }

    if (improvement_type == ImprovementType::BI && b1_best != -1) {
        sol->moveItem(a_best, b1_best, b2_best);
        sol->moveItem(b_best, b1_best, b2_best);
        sol->moveItem(c_best, b2_best, b1_best);
        sol->moveItem(d_best, b2_best, b1_best);

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Exchange 32 (delta version) --------------------
bool LocalSearch::exchange32() {
    if (sol->isFeasible()) {
        return false;
    }

    int best_delta = 0;

    int b1_best = -1, b2_best = -1;
    int a_best = -1, b_best = -1, c_best = -1;
    int d_best = -1, e_best = -1;

    for (int b1 : sol->bad_bins) {
        const auto& bin1 = sol->bins[b1];
        int sz1 = (int)bin1.size();

        if (sz1 < 3) continue;

        for (int i1 = 0; i1 < sz1; ++i1) {
            for (int i2 = i1 + 1; i2 < sz1; ++i2) {
                for (int i3 = i2 + 1; i3 < sz1; ++i3) {

                    std::vector<int> subset1 = {
                        sol->bins[b1][i1],
                        sol->bins[b1][i2],
                        sol->bins[b1][i3]
                    };

                    for (int b2 = 0; b2 < (int)sol->bins.size(); ++b2) {
                        if (b2 == b1) continue;

                        const auto& bin2 = sol->bins[b2];
                        int sz2 = (int)bin2.size();

                        if (sz2 < 2) continue;

                        for (int j1 = 0; j1 < sz2; ++j1) {
                            for (int j2 = j1 + 1; j2 < sz2; ++j2) {

                                std::vector<int> subset2 = {
                                    sol->bins[b2][j1],
                                    sol->bins[b2][j2]
                                };

                                int delta = sol->deltaSwapSubset(
                                    b1, subset1,
                                    b2, subset2,
                                    K1, K2, K3
                                );

                                if (improvement_type == ImprovementType::FI && delta < 0) {
                                    int a = subset1[0];
                                    int b = subset1[1];
                                    int c = subset1[2];
                                    int d = subset2[0];
                                    int e = subset2[1];

                                    sol->moveItem(a, b1, b2);
                                    sol->moveItem(b, b1, b2);
                                    sol->moveItem(c, b1, b2);
                                    sol->moveItem(d, b2, b1);
                                    sol->moveItem(e, b2, b1);

                                    sol->removeEmptyBins();
                                    return true;
                                }

                                if (delta < best_delta) {
                                    best_delta = delta;

                                    b1_best = b1;
                                    b2_best = b2;

                                    a_best = subset1[0];
                                    b_best = subset1[1];
                                    c_best = subset1[2];
                                    d_best = subset2[0];
                                    e_best = subset2[1];
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (improvement_type == ImprovementType::BI && b1_best != -1) {
        sol->moveItem(a_best, b1_best, b2_best);
        sol->moveItem(b_best, b1_best, b2_best);
        sol->moveItem(c_best, b1_best, b2_best);
        sol->moveItem(d_best, b2_best, b1_best);
        sol->moveItem(e_best, b2_best, b1_best);

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Best Move for Pair (delta version) --------------------
bool LocalSearch::bestMoveForPair(int b1, int b2, bool allow_zero_cost) {
    if (b1 == b2) return false;
    if (b1 < 0 || b2 < 0) return false;
    if (b1 >= (int)sol->bins.size() || b2 >= (int)sol->bins.size()) return false;

    enum class MoveType {
        NONE,
        RELOCATE_12,
        RELOCATE_21,
        SWAP,
        SWAP21_12,
        SWAP21_21
    };

    struct BestMove {
        MoveType type = MoveType::NONE;
        int delta = INT_MAX;

        // generic indices / item ids
        int item = -1;     // for relocate
        int i = -1, j = -1; // for swap indices
        int i1 = -1, i2 = -1, j1 = -1; // for 2-vs-1 indices

        // item ids for safe application of 2-vs-1
        int a = -1, b = -1, c = -1;
    };

    auto acceptable = [&](int delta) {
        return (delta < 0) || (allow_zero_cost && delta == 0);
    };

    BestMove best;

    // -------------------- Relocate b1 -> b2 --------------------
    for (int item : sol->bins[b1]) {
        int delta = sol->deltaMove(item, b1, b2, K1, K2, K3);
        if (!acceptable(delta)) continue;

        if (delta < best.delta) {
            best.delta = delta;
            best.type = MoveType::RELOCATE_12;
            best.item = item;
        }

        if (improvement_type == ImprovementType::FI) {
            sol->moveItem(item, b1, b2);
            sol->removeEmptyBins();
            return true;
        }
    }

    // -------------------- Relocate b2 -> b1 --------------------
    for (int item : sol->bins[b2]) {
        int delta = sol->deltaMove(item, b2, b1, K1, K2, K3);
        if (!acceptable(delta)) continue;

        if (delta < best.delta) {
            best.delta = delta;
            best.type = MoveType::RELOCATE_21;
            best.item = item;
        }

        if (improvement_type == ImprovementType::FI) {
            sol->moveItem(item, b2, b1);
            sol->removeEmptyBins();
            return true;
        }
    }

    // -------------------- Swap --------------------
    for (int i = 0; i < (int)sol->bins[b1].size(); ++i) {
        for (int j = 0; j < (int)sol->bins[b2].size(); ++j) {
            int delta = sol->deltaSwap(b1, i, b2, j, K1, K2, K3);
            if (!acceptable(delta)) continue;

            if (delta < best.delta) {
                best.delta = delta;
                best.type = MoveType::SWAP;
                best.i = i;
                best.j = j;
            }

            if (improvement_type == ImprovementType::FI) {
                sol->swapItems(b1, i, b2, j);
                sol->removeEmptyBins();
                return true;
            }
        }
    }

    // -------------------- 2-vs-1 from b1 -> b2 --------------------
    if ((int)sol->bins[b1].size() >= 2 && !sol->bins[b2].empty()) {
        for (int i1 = 0; i1 < (int)sol->bins[b1].size(); ++i1) {
            for (int i2 = i1 + 1; i2 < (int)sol->bins[b1].size(); ++i2) {
                for (int j = 0; j < (int)sol->bins[b2].size(); ++j) {
                    int delta = sol->deltaSwap21(b1, i1, i2, b2, j, K1, K2, K3);
                    if (!acceptable(delta)) continue;

                    int a = sol->bins[b1][i1];
                    int b = sol->bins[b1][i2];
                    int c = sol->bins[b2][j];

                    if (delta < best.delta) {
                        best.delta = delta;
                        best.type = MoveType::SWAP21_12;
                        best.i1 = i1;
                        best.i2 = i2;
                        best.j1 = j;
                        best.a = a;
                        best.b = b;
                        best.c = c;
                    }

                    if (improvement_type == ImprovementType::FI) {
                        sol->moveItem(a, b1, b2);
                        sol->moveItem(b, b1, b2);
                        sol->moveItem(c, b2, b1);
                        sol->removeEmptyBins();
                        return true;
                    }
                }
            }
        }
    }

    // -------------------- 2-vs-1 from b2 -> b1 --------------------
    if ((int)sol->bins[b2].size() >= 2 && !sol->bins[b1].empty()) {
        for (int i1 = 0; i1 < (int)sol->bins[b2].size(); ++i1) {
            for (int i2 = i1 + 1; i2 < (int)sol->bins[b2].size(); ++i2) {
                for (int j = 0; j < (int)sol->bins[b1].size(); ++j) {
                    int delta = sol->deltaSwap21(b2, i1, i2, b1, j, K1, K2, K3);
                    if (!acceptable(delta)) continue;

                    int a = sol->bins[b2][i1];
                    int b = sol->bins[b2][i2];
                    int c = sol->bins[b1][j];

                    if (delta < best.delta) {
                        best.delta = delta;
                        best.type = MoveType::SWAP21_21;
                        best.i1 = i1;
                        best.i2 = i2;
                        best.j1 = j;
                        best.a = a;
                        best.b = b;
                        best.c = c;
                    }

                    if (improvement_type == ImprovementType::FI) {
                        sol->moveItem(a, b2, b1);
                        sol->moveItem(b, b2, b1);
                        sol->moveItem(c, b1, b2);
                        sol->removeEmptyBins();
                        return true;
                    }
                }
            }
        }
    }

    // -------------------- Apply best improvement --------------------
    if (improvement_type == ImprovementType::BI && best.type != MoveType::NONE) {
        switch (best.type) {
            case MoveType::RELOCATE_12:
                sol->moveItem(best.item, b1, b2);
                break;

            case MoveType::RELOCATE_21:
                sol->moveItem(best.item, b2, b1);
                break;

            case MoveType::SWAP:
                sol->swapItems(b1, best.i, b2, best.j);
                break;

            case MoveType::SWAP21_12:
                sol->moveItem(best.a, b1, b2);
                sol->moveItem(best.b, b1, b2);
                sol->moveItem(best.c, b2, b1);
                break;

            case MoveType::SWAP21_21:
                sol->moveItem(best.a, b2, b1);
                sol->moveItem(best.b, b2, b1);
                sol->moveItem(best.c, b1, b2);
                break;

            case MoveType::NONE:
                return false;
        }

        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- One iteration of classic local search only --------------------
// bool LocalSearch::classic() {
//     if (sol->isFeasible()) {
//         return false;
//     }

//     std::vector<int> order = {0, 1, 2};
//     std::shuffle(order.begin(), order.end(), rng);

//     bool improved = false;

//     for (int op : order) {
//         switch (op) {
//             case 0: improved |= relocation(); break;
//             case 1: improved |= exchange(); break;
//             case 2: improved |= exchange21(); break;
//         }
//     }

//     return improved;
// }

bool LocalSearch::classic() {
    if (sol->isFeasible()) {
        return false;
    }

    bool overall_improved = false;

    std::vector<int> order = {0, 1, 2};
    std::shuffle(order.begin(), order.end(), rng);

    while (!sol->isFeasible()) {
        bool improved_this_loop = false;

        for (int op : order) {
            bool moved = false;

            switch (op) {
                case 0:
                    moved = relocation();
                    break;
                case 1:
                    moved = exchange();
                    break;
                case 2:
                    moved = exchange21();
                    break;
            }

            if (moved) {
                improved_this_loop = true;
                overall_improved = true;
            }

            if (sol->isFeasible()) {
                break;
            }
        }

        if (!improved_this_loop) {
            break;
        }
    }

    return overall_improved;
}

bool LocalSearch::classic_ILS() {
    if (sol->isFeasible()) {
        return false;
    }

    auto phi = [&](int b) -> int {
        int excess = std::max(0, sol->bin_loads[b] - sol->C);
        return K2 * excess + K3 * sol->bin_conflicts[b];
    };

    bool overall_improved = false;
    int it_loop = 0;

    std::vector<int> bin_version(sol->bins.size(), 0);
    std::vector<std::vector<int>> last_eval_v1(
        sol->bins.size(), std::vector<int>(sol->bins.size(), -1));
    std::vector<std::vector<int>> last_eval_v2(
        sol->bins.size(), std::vector<int>(sol->bins.size(), -1));

    while (true) {
        ++it_loop;
        bool allow_zero_cost = (it_loop == 1);
        bool moved_in_this_loop = false;
        bool restart_loop = false;

        std::vector<int> bins_order(sol->bins.size());
        std::iota(bins_order.begin(), bins_order.end(), 0);
        std::shuffle(bins_order.begin(), bins_order.end(), rng);

        for (int b1 : bins_order) {
            if (restart_loop) break;
            if (b1 >= (int)sol->bins.size()) continue;
            if (phi(b1) <= 0) continue;

            std::vector<int> bins_order_2(sol->bins.size());
            std::iota(bins_order_2.begin(), bins_order_2.end(), 0);
            std::shuffle(bins_order_2.begin(), bins_order_2.end(), rng);

            for (int b2 : bins_order_2) {
                if (b2 == b1) continue;
                if (b1 >= (int)sol->bins.size() || b2 >= (int)sol->bins.size()) continue;

                bool must_evaluate =
                    (it_loop == 1) ||
                    (last_eval_v1[b1][b2] != bin_version[b1]) ||
                    (last_eval_v2[b1][b2] != bin_version[b2]);

                if (!must_evaluate) continue;

                int old_n_bins = (int)sol->bins.size();
                bool moved = bestMoveForPair(b1, b2, allow_zero_cost);
                int new_n_bins = (int)sol->bins.size();

                if (moved) {
                    overall_improved = true;
                    moved_in_this_loop = true;

                    if (new_n_bins != old_n_bins) {
                        // Bin indices may have changed because removeEmptyBins remapped them.
                        // Rebuild all loop bookkeeping and restart from scratch.
                        bin_version.assign(sol->bins.size(), 0);
                        last_eval_v1.assign(
                            sol->bins.size(),
                            std::vector<int>(sol->bins.size(), -1));
                        last_eval_v2.assign(
                            sol->bins.size(),
                            std::vector<int>(sol->bins.size(), -1));

                        restart_loop = true;
                        break;
                    } else {
                        // Indices are stable; mark these bins as changed.
                        ++bin_version[b1];
                        ++bin_version[b2];
                    }

                    if (improvement_type == ImprovementType::FI) {
                        restart_loop = true;
                        break;
                    }
                } else {
                    last_eval_v1[b1][b2] = bin_version[b1];
                    last_eval_v2[b1][b2] = bin_version[b2];
                }
            }
        }

        if (!moved_in_this_loop || sol->isFeasible()) {
            break;
        }
    }

    return overall_improved;
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

    // -------------------- Tabu check --------------------
    size_t candidate_hash = hashSolution(candidate.bins);

    if (isTabu(candidate_hash) && obj_after >= obj_before) {
        return false;
    }

    // -------------------- Accept if improving or equal --------------------
    if (obj_after <= obj_before) {
        *sol = candidate;

        addTabu(candidate_hash);
        last_solution_hash = candidate_hash;

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

bool LocalSearch::setCoveringLPFeasible(int K_upper) {
    try {
        const int n_items = sol->N;
        const int n_cols  = static_cast<int>(pool.size());

        if (pool.empty()) return false;

        GRBEnv env(true);
        env.set("OutputFlag", "0");
        env.start();

        GRBModel model(env);

        model.set(GRB_DoubleParam_TimeLimit, 5.0);

        std::vector<GRBVar> x(n_cols);
        for (int j = 0; j < n_cols; ++j) {
            x[j] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
        }

        std::vector<GRBLinExpr> cover_constr(n_items);
        GRBLinExpr k_constr = 0;

        for (int i = 0; i < n_items; ++i)
            cover_constr[i] = 0;

        for (int j = 0; j < n_cols; ++j) {
            const auto& col = pool[j].col;

            for (int i = 0; i < n_items; ++i) {
                if (col.mask[i]) {
                    cover_constr[i] += x[j];
                }
            }

            k_constr += x[j];
        }

        for (int i = 0; i < n_items; ++i) {
            model.addConstr(cover_constr[i] >= 1);
        }

        model.addConstr(k_constr <= K_upper);

        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        // LP infeasible => MIP infeasible
        if (status == GRB_INFEASIBLE) {
            return false;
        }

        // Otherwise assume feasible (OPTIMAL or TIME_LIMIT etc.)
        return true;

    } catch (...) {
        return true; // fail-open (don’t block MIP)
    }
}

// -------------------- Set Covering --------------------
bool LocalSearch::setCoveringVanilla() {
    bool verbose = false;

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    const int GUROBI_TIME_LIMIT_MS = 30000;
    const int WATCHDOG_LIMIT_MS    = 35000;
    const int CHECK_INTERVAL_MS    = 1000;

    log("\n[SC] START setCovering()");

    // std::shuffle(pool.begin(), pool.end(), rng);

    if (pool.empty()) {
        log("[SC] FAILED: empty pool");
        return false;
    }

    const int n_items = sol->N;
    const int n_cols  = static_cast<int>(pool.size());
    int K_upper = K;

    if (sol->isFeasible() && K_upper > 1) {
        K_upper -= 1;
        log("[SC] MODE: compression (K-1)");
    } else {
        log("[SC] MODE: repair (K)");
    }

    if (!setCoveringLPFeasible(K_upper)) {
        log("[SC] LP infeasible -> skipping MIP");
        return false;
    }

    log("[SC] n_items=" + std::to_string(n_items) +
        " n_cols=" + std::to_string(n_cols) +
        " K_upper=" + std::to_string(K_upper));

    try {
        GRBEnv env(true);
        env.set("OutputFlag", "0");
        env.start();

        GRBModel model(env);

        model.set(GRB_DoubleParam_TimeLimit, GUROBI_TIME_LIMIT_MS / 1000.0);
        model.set(GRB_IntParam_MIPFocus, 1);

        std::vector<GRBVar> x(n_cols);
        for (int j = 0; j < n_cols; ++j) {
            x[j] = model.addVar(0.0, 1.0, pool[j].col.cost, GRB_BINARY);
        }

        std::vector<GRBLinExpr> cover_constr(n_items);
        GRBLinExpr k_constr = 0;

        for (int i = 0; i < n_items; ++i) {
            cover_constr[i] = 0;
        }

        for (int j = 0; j < n_cols; ++j) {
            const auto& col = pool[j].col;

            for (int i = 0; i < n_items; ++i) {
                if (col.mask[i]) {
                    cover_constr[i] += x[j];
                }
            }

            k_constr += x[j];
        }

        for (int i = 0; i < n_items; ++i) {
            model.addConstr(cover_constr[i] >= 1);
        }

        model.addConstr(k_constr <= K_upper);

        if (K_upper == K &&
            !elite_bins.empty() &&
            elite_bins.size() <= static_cast<size_t>(K_upper)) {
            int start_count = 0;

            for (int j = 0; j < n_cols; ++j) {
                if (elite_hashes.count(pool[j].hash)) {
                    x[j].set(GRB_DoubleAttr_Start, 1.0);
                    start_count++;
                } else {
                    x[j].set(GRB_DoubleAttr_Start, 0.0);
                }
            }

            model.set(GRB_IntParam_MIPFocus, 2);

            log("[SC] injected elite MIP start with " + std::to_string(start_count) + " bins");
        } else {
            log("[SC] skipped elite MIP start");
        }

        model.update();

        std::atomic<bool> finished(false);

        std::thread watchdog([&]() {
            int waited_ms = 0;

            while (waited_ms < WATCHDOG_LIMIT_MS) {
                if (finished.load()) {
                    log("[SC] Watchdog exiting normally");
                    return;
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(CHECK_INTERVAL_MS)
                );
                waited_ms += CHECK_INTERVAL_MS;
            }

            if (!finished.load()) {
                log("[SC] Watchdog triggered -> terminating Gurobi");
                try {
                    model.terminate();
                } catch (...) {}
            }
        });

        log("[SC] calling Gurobi...");

        auto t0 = std::chrono::high_resolution_clock::now();
        model.optimize();
        auto t1 = std::chrono::high_resolution_clock::now();

        double solve_time = std::chrono::duration<double>(t1 - t0).count();
        log("[SC] Gurobi solve time = " + std::to_string(solve_time) + " s");

        finished = true;

        if (watchdog.joinable()) {
            watchdog.join();
        }

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL) {
            log("[SC] optimal solution found");
            updatePoolSize(true);
        }

        if (status == GRB_TIME_LIMIT) {
            log("[SC] time limit reached");
            updatePoolSize(false);
        } else if (status == GRB_INTERRUPTED) {
            log("[SC] interrupted by watchdog");
        }

        if (status == GRB_INFEASIBLE) {
            log("[SC] infeasible restricted pool");
            return false;
        }

        if (model.get(GRB_IntAttr_SolCount) == 0) {
            log("[SC] no feasible solution found");
            return false;
        }

        if (status != GRB_OPTIMAL &&
            status != GRB_TIME_LIMIT &&
            status != GRB_INTERRUPTED) {
            log("[SC] solver error");
            return false;
        }

        std::vector<std::vector<int>> new_bins;
        new_bins.reserve(K);

        for (int j = 0; j < n_cols; ++j) {
            if (x[j].get(GRB_DoubleAttr_X) > 0.5) {
                new_bins.push_back(pool[j].col.items);
            }
        }

        if (new_bins.empty()) {
            log("[SC] no solution found");
            return false;
        }

        std::vector<std::vector<int>> repaired_bins = repairSolution(new_bins);

        size_t new_hash = hashSolution(repaired_bins);
        size_t old_hash = hashSolution(sol->bins);

        if (new_hash == old_hash) {
            log("[SC] identical solution. Rejecting.");
            return false;
        }

        if (isTabu(new_hash)) {
            log("[SC] TABU solution detected. Rejecting.");
            return false;
        }

        int old_obj = sol->computeObjective(K1, K2, K3);

        BPPCSolution temp_sol = *sol;
        temp_sol.rebuildSolutionFromBins(repaired_bins);
        temp_sol.removeEmptyBins();

        int new_obj = temp_sol.computeObjective(K1, K2, K3);

        log("[SC] old_obj= " + std::to_string(old_obj));
        log("[SC] new_obj= " + std::to_string(new_obj));

        if (new_obj >= old_obj) {
            log("[SC] didn't improve objective value solution. Rejecting.");
            return false;
        }

        log("[SC] improved objective value solution. Accepting.");

        *sol = temp_sol;
        
        addTabu(new_hash);
        last_solution_hash = new_hash;

        log("[SC] SUCCESS");
        return true;

    } catch (GRBException& e) {
        log("[SC] Gurobi error: " + std::string(e.getMessage()));
        return false;
    } catch (...) {
        log("[SC] Unknown Gurobi error");
        return false;
    }
}

bool LocalSearch::setCoveringBinFeasible() {
    bool verbose = false;

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    const int GUROBI_TIME_LIMIT_MS = 30000;
    const int WATCHDOG_LIMIT_MS    = 35000;
    const int CHECK_INTERVAL_MS    = 1000;

    log("\n[SC] START setCovering()");

    if (pool.empty()) {
        log("[SC] FAILED: empty pool");
        return false;
    }

    const int n_items = sol->N;
    const int n_cols  = static_cast<int>(pool.size());

    log("[SC] n_items=" + std::to_string(n_items) +
        " n_cols=" + std::to_string(n_cols));

    try {
        GRBEnv env(true);
        env.set("OutputFlag", "0");
        env.start();

        GRBModel model(env);

        model.set(GRB_DoubleParam_TimeLimit, GUROBI_TIME_LIMIT_MS / 1000.0);

        std::vector<GRBVar> x(n_cols);
        for (int j = 0; j < n_cols; ++j) {
            // Objective = minimize number of selected bins
            x[j] = model.addVar(0.0, 1.0, 1.0, GRB_BINARY);
        }

        std::vector<GRBLinExpr> cover_constr(n_items);
        GRBLinExpr k_constr = 0;

        for (int i = 0; i < n_items; ++i) {
            cover_constr[i] = 0;
        }

        for (int j = 0; j < n_cols; ++j) {
            const auto& col = pool[j].col;

            for (int i = 0; i < n_items; ++i) {
                if (col.mask[i]) {
                    cover_constr[i] += x[j];
                }
            }

            k_constr += x[j];
        }

        // Cover all items
        for (int i = 0; i < n_items; ++i) {
            model.addConstr(cover_constr[i] >= 1);
        }

        // MIP start from elite solution if available and compatible
        if (!elite_bins.empty()) {

            int start_count = 0;
            for (int j = 0; j < n_cols; ++j) {
                if (elite_hashes.count(pool[j].hash)) {
                    x[j].set(GRB_DoubleAttr_Start, 1.0);
                    start_count++;
                } else {
                    x[j].set(GRB_DoubleAttr_Start, 0.0);
                }
            }

            // Since we already have a start, focus more on improvement
            model.set(GRB_IntParam_MIPFocus, 2);

            log("[SC] injected elite MIP start with " +
                std::to_string(start_count) + " bins");
        }

        model.update();

        std::atomic<bool> finished(false);

        std::thread watchdog([&]() {
            int waited_ms = 0;

            while (waited_ms < WATCHDOG_LIMIT_MS) {
                if (finished.load()) {
                    log("[SC] Watchdog exiting normally");
                    return;
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(CHECK_INTERVAL_MS)
                );
                waited_ms += CHECK_INTERVAL_MS;
            }

            if (!finished.load()) {
                log("[SC] Watchdog triggered -> terminating Gurobi");
                try {
                    model.terminate();
                } catch (...) {}
            }
        });

        log("[SC] calling Gurobi...");

        auto t0 = std::chrono::high_resolution_clock::now();
        model.optimize();
        auto t1 = std::chrono::high_resolution_clock::now();

        double solve_time = std::chrono::duration<double>(t1 - t0).count();
        log("[SC] Gurobi solve time = " + std::to_string(solve_time) + " s");

        finished = true;

        if (watchdog.joinable()) {
            watchdog.join();
        }

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL) {
            log("[SC] optimal solution found");
            updatePoolSize(true);
        }

        if (status == GRB_TIME_LIMIT) {
            log("[SC] time limit reached");
            updatePoolSize(false);
        } else if (status == GRB_INTERRUPTED) {
            log("[SC] interrupted by watchdog");
        }

        if (status == GRB_INFEASIBLE) {
            log("[SC] infeasible restricted pool");
            return false;
        }

        if (model.get(GRB_IntAttr_SolCount) == 0) {
            log("[SC] no feasible solution found");
            return false;
        }

        if (status != GRB_OPTIMAL &&
            status != GRB_TIME_LIMIT &&
            status != GRB_INTERRUPTED) {
            log("[SC] solver error");
            return false;
        }

        // Extract selected bins
        std::vector<std::vector<int>> new_bins;
        new_bins.reserve(K);

        for (int j = 0; j < n_cols; ++j) {
            if (x[j].get(GRB_DoubleAttr_X) > 0.5) {
                new_bins.push_back(pool[j].col.items);
            }
        }

        if (new_bins.empty()) {
            log("[SC] no solution found");
            return false;
        }

        // Postprocess duplicates
        std::vector<std::vector<int>> repaired_bins = repairSolution(new_bins);

        size_t new_hash = hashSolution(repaired_bins);
        size_t old_hash = hashSolution(sol->bins);

        if (new_hash == old_hash) {
            log("[SC] identical solution. Rejecting.");
            return false;
        }

        if (isTabu(new_hash)) {
            log("[SC] TABU solution detected. Rejecting.");
            return false;
        }

        int old_obj = sol->computeObjective(K1, K2, K3);

        BPPCSolution temp_sol = *sol;
        temp_sol.rebuildSolutionFromBins(repaired_bins);
        temp_sol.removeEmptyBins();

        int new_obj = temp_sol.computeObjective(K1, K2, K3);

        log("[SC] old_obj= " + std::to_string(old_obj));
        log("[SC] new_obj= " + std::to_string(new_obj));

        if (new_obj >= old_obj) {
            log("[SC] didn't improve objective value solution. Rejecting.");
            return false;
        }

        log("[SC] improved objective value solution. Accepting.");

        *sol = temp_sol;
        
        addTabu(new_hash);
        last_solution_hash = new_hash;

        log("[SC] SUCCESS");
        return true;

    } catch (GRBException& e) {
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

void LocalSearch::updateElite() {
    if (K > sol->bins_used && sol->isFeasible()) {
        elite_bins = sol->bins;

        elite_hashes.clear();
        for (const auto& bin : elite_bins) {
            elite_hashes.insert(hashBin(bin));
        }

        elite_solution_hash = hashSolution(elite_bins);
    }
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

// -------------------- Add Column: Allow Infeasible --------------------
inline void LocalSearch::addColumnAllowInfeasible(Column&& col, int n_items) {
    auto isSubset = [](const Column& a, const Column& b) {
        for (size_t i = 0; i < a.mask.size(); ++i) {
            if (a.mask[i] && !b.mask[i]) return false;
        }
        return true;
    };

    auto isEqualMask = [](const Column& a, const Column& b) {
        return a.mask == b.mask;
    };

    auto isFeasibleCol = [&](const Column& c) {
        return c.cost == K1;
    };

    col.buildMask(n_items);
    size_t h = hashBin(col.items);

    // Exact duplicate
    if (seen.count(h)) return;

    // If new column is a subset of an existing one:
    // - normally reject it
    // - but if it is currently elite and the existing superset is feasible,
    //   replace the elite reference by the superset
    for (const auto& entry : pool) {
        if (isSubset(col, entry.col)) {
            bool new_is_elite = elite_hashes.count(h);
            bool existing_is_feasible = isFeasibleCol(entry.col);
            bool existing_already_elite = elite_hashes.count(entry.hash);

            if (new_is_elite && existing_is_feasible && !existing_already_elite) {
                // Replace elite bin/hash h by entry.hash
                for (auto& elite_bin : elite_bins) {
                    if (hashBin(elite_bin) == h) {
                        elite_hashes.erase(h);
                        elite_bin = entry.col.items;
                        elite_hashes.insert(entry.hash);
                        elite_solution_hash = hashSolution(elite_bins);
                        break;
                    }
                }
            }
            return;
        }
    }

    // Remove existing non-elite columns that are strict subsets of the new one
    for (auto it = pool.begin(); it != pool.end(); ) {
        bool old_subset_of_new = isSubset(it->col, col);
        bool equal_masks = isEqualMask(it->col, col);

        if (old_subset_of_new && !equal_masks && !elite_hashes.count(it->hash)) {
            seen.erase(it->hash);
            it = pool.erase(it);
        } else {
            ++it;
        }
    }

    PoolEntry entry;
    entry.col = std::move(col);
    entry.hash = h;

    pool.push_back(std::move(entry));
    seen.insert(h);
}

// -------------------- Add Column: Only Feasible --------------------
inline void LocalSearch::addColumn(Column&& col, int n_items) {
    auto isSubset = [](const Column& a, const Column& b) {
        for (size_t i = 0; i < a.mask.size(); ++i) {
            if (a.mask[i] && !b.mask[i]) return false;
        }
        return true;
    };

    auto isEqualMask = [](const Column& a, const Column& b) {
        return a.mask == b.mask;
    };

    auto isFeasibleCol = [&](const Column& c) {
        return c.cost == K1;
    };

    col.buildMask(n_items);
    size_t h = hashBin(col.items);

    // Only keep feasible columns
    if (!isFeasibleCol(col)) return;

    // Exact duplicate
    if (seen.count(h)) return;

    // If new feasible column is a subset of an existing one:
    // - normally reject it
    // - but if it is currently elite and the existing superset is feasible,
    //   replace the elite reference by the superset
    for (const auto& entry : pool) {
        if (isSubset(col, entry.col)) {
            bool new_is_elite = elite_hashes.count(h);
            bool existing_is_feasible = isFeasibleCol(entry.col);
            bool existing_already_elite = elite_hashes.count(entry.hash);

            if (new_is_elite && existing_is_feasible && !existing_already_elite) {
                // Replace elite bin/hash h by entry.hash
                for (auto& elite_bin : elite_bins) {
                    if (hashBin(elite_bin) == h) {
                        elite_hashes.erase(h);
                        elite_bin = entry.col.items;
                        elite_hashes.insert(entry.hash);
                        elite_solution_hash = hashSolution(elite_bins);
                        break;
                    }
                }
            }
            return;
        }
    }

    // Remove existing non-elite columns that are strict subsets of the new one
    for (auto it = pool.begin(); it != pool.end(); ) {
        bool old_subset_of_new = isSubset(it->col, col);
        bool equal_masks = isEqualMask(it->col, col);

        if (old_subset_of_new && !equal_masks && !elite_hashes.count(it->hash)) {
            seen.erase(it->hash);
            it = pool.erase(it);
        } else {
            ++it;
        }
    }

    PoolEntry entry;
    entry.col = std::move(col);
    entry.hash = h;

    pool.push_back(std::move(entry));
    seen.insert(h);
}

void LocalSearch::addToPool(const BPPCSolution& s) {
    const int n_items = s.N;

    for (int b = 0; b < (int)s.bins.size(); b++) {
        const auto& bin = s.bins[b];
        if (bin.empty()) continue;

        Column col;
        col.items = bin;

        int load = s.bin_loads[b];
        int conflicts = s.bin_conflicts[b];
        int excess = std::max(0, load - s.C);

        col.cost = K1 + K2 * excess + K3 * conflicts;

        addColumn(std::move(col), n_items);
    }

    trimPool();
}

void LocalSearch::trimPool() {
    if ((int)pool.size() <= S_pool) return;

    // -------------------- First remove infeasible non-elite columns --------------------
    auto it = pool.begin();
    while ((int)pool.size() > S_pool && it != pool.end()) {
        bool is_feasible_bin = (it->col.cost == K1);

        if (!is_feasible_bin && !elite_hashes.count(it->hash)) {
            seen.erase(it->hash);
            it = pool.erase(it);
        } else {
            ++it;
        }
    }

    if ((int)pool.size() <= S_pool) return;

    // -------------------- Then trim oldest non-elite columns --------------------
    it = pool.begin();
    while ((int)pool.size() > S_pool && it != pool.end()) {
        if (!elite_hashes.count(it->hash)) {
            seen.erase(it->hash);
            it = pool.erase(it);
        } else {
            ++it;
        }
    }
}

void LocalSearch::updatePoolSize(bool optimal_solved) {

    if (optimal_solved) {
        S_pool = (int)(S_pool * 1.15);
    } else {
        S_pool = (int)(S_pool * 0.85);
    }

    // S_pool = std::max(S_pool, S_min);
    // S_pool = std::min(S_pool, S_max);
}

// -------------------- Objective --------------------
int LocalSearch::computeObjective(const BPPCSolution& s) const {
    return s.computeObjective(K1, K2, K3);
}

inline size_t LocalSearch::hashBin(const std::vector<int>& bin) const {
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

inline size_t LocalSearch::hashSolution(const std::vector<std::vector<int>>& bins) const {
    static constexpr uint64_t FNV = 1469598103934665603ULL;
    static constexpr uint64_t PRIME = 1099511628211ULL;

    std::vector<size_t> bin_hashes;
    bin_hashes.reserve(bins.size());

    for (const auto& bin : bins)
    {
        bin_hashes.push_back(hashBin(bin));
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

// std::vector<std::vector<int>> LocalSearch::repairSolution(
//     const std::vector<std::vector<int>>& input_bins) {
//     std::vector<std::vector<int>> bins = input_bins;

//     const int n_items = sol->N;

//     // Build item to bins structure
//     std::vector<std::vector<int>> item_bins(n_items);

//     for (int b = 0; b < (int)bins.size(); b++) {
//         for (int item : bins[b]) {
//             item_bins[item].push_back(b);
//         }
//     }

//     // Extract duplicated items only
//     std::vector<int> dup_items;
//     for (int i = 0; i < n_items; i++) {
//         if (item_bins[i].size() > 1) {
//             dup_items.push_back(i);
//         }
//     }

//     // Best solution tracking
//     std::vector<std::vector<int>> best_bins;
//     int best_cost = std::numeric_limits<int>::max();

//     // DFS recursion
//     std::function<void(int)> dfs = [&](int idx)
//     {
//         // Leaf: all duplicates resolved
//         if (idx == (int)dup_items.size()) {

//             int cost = 0;
//             for (auto& b : bins)
//                 if (!b.empty()) cost++;

//             if (cost < best_cost) {
//                 best_cost = cost;
//                 best_bins = bins;
//             }

//             return;
//         }

//         int item = dup_items[idx];
//         const auto& cand_bins = item_bins[item];

//         // Try each bin as "keep bin"
//         for (int keep_bin : cand_bins) {

//             // Store removed positions for rollback
//             std::vector<std::pair<int, int>> removed;

//             // Remove item from all other bins
//             for (int b : cand_bins) {
//                 if (b == keep_bin) continue;

//                 auto& bin = bins[b];

//                 auto it = std::find(bin.begin(), bin.end(), item);
//                 if (it != bin.end()) {
//                     bin.erase(it);
//                     removed.push_back({b, item});
//                 }
//             }

//             dfs(idx + 1);

//             // Rollback
//             for (auto& [b, val] : removed) {
//                 bins[b].push_back(val);
//             }
//         }
//     };

//     // Run DP/DFS
//     dfs(0);

//     return best_bins;
// }

std::vector<std::vector<int>> LocalSearch::repairSolution(
    const std::vector<std::vector<int>>& input_bins) {

    const int n_items = sol->N;
    const int n_bins  = (int)input_bins.size();

    if (n_bins == 0) return input_bins;

    // ---------------- Build item -> bins ----------------
    std::vector<std::vector<int>> item_bins(n_items);
    std::vector<int> bin_size(n_bins, 0);

    for (int b = 0; b < n_bins; b++) {
        bin_size[b] = (int)input_bins[b].size();
        for (int item : input_bins[b]) {
            item_bins[item].push_back(b);
        }
    }

    // ---------------- Duplicated items only ----------------
    std::vector<int> dup_items;
    dup_items.reserve(n_items);

    for (int i = 0; i < n_items; i++) {
        if ((int)item_bins[i].size() > 1) {
            dup_items.push_back(i);
        }
    }

    if (dup_items.empty()) {
        return input_bins;
    }

    // Most constrained first
    std::sort(dup_items.begin(), dup_items.end(),
              [&](int a, int b) {
                  return item_bins[a].size() < item_bins[b].size();
              });

    const int D = (int)dup_items.size();

    // For each bin: how many unresolved duplicated items are currently in it
    std::vector<int> dup_count_in_bin(n_bins, 0);
    for (int item : dup_items) {
        for (int b : item_bins[item]) {
            dup_count_in_bin[b]++;
        }
    }

    // Chosen keep-bin for each duplicated item
    std::vector<int> chosen_bin(D, -1);
    std::vector<int> best_choice(D, -1);

    int best_cost = std::numeric_limits<int>::max();

    auto count_nonempty = [&](const std::vector<int>& sz) {
        int cnt = 0;
        for (int x : sz) {
            if (x > 0) cnt++;
        }
        return cnt;
    };

    // Exact lower bound:
    // a nonempty bin can still become empty only if all remaining items in it
    // are unresolved duplicated items.
    auto lower_bound_cost = [&](const std::vector<int>& cur_bin_size,
                                const std::vector<int>& cur_dup_count_in_bin) {
        int nonempty = 0;
        int maybe_emptyable = 0;

        for (int b = 0; b < n_bins; b++) {
            if (cur_bin_size[b] > 0) {
                nonempty++;
                if (cur_bin_size[b] == cur_dup_count_in_bin[b]) {
                    maybe_emptyable++;
                }
            }
        }

        return nonempty - maybe_emptyable;
    };

    std::function<void(int, std::vector<int>&, std::vector<int>&)> dfs =
        [&](int idx, std::vector<int>& cur_bin_size, std::vector<int>& cur_dup_count_in_bin) {

        int lb = lower_bound_cost(cur_bin_size, cur_dup_count_in_bin);
        if (lb >= best_cost) return;

        if (idx == D) {
            int cost = count_nonempty(cur_bin_size);
            if (cost < best_cost) {
                best_cost = cost;
                best_choice = chosen_bin;
            }
            return;
        }

        int item = dup_items[idx];
        const auto& cand_bins = item_bins[item];

        // Try bins that look more promising first:
        // prefer keeping the item in bins that currently have more items
        std::vector<int> ordered = cand_bins;
        std::sort(ordered.begin(), ordered.end(),
                  [&](int a, int b) {
                      return cur_bin_size[a] > cur_bin_size[b];
                  });

        for (int keep_bin : ordered) {
            chosen_bin[idx] = keep_bin;

            // Remove this duplicated item from all other bins that contain it
            for (int b : cand_bins) {
                if (b != keep_bin) {
                    cur_bin_size[b]--;
                }
            }

            // This duplicated item is no longer unresolved in any candidate bin
            for (int b : cand_bins) {
                cur_dup_count_in_bin[b]--;
            }

            dfs(idx + 1, cur_bin_size, cur_dup_count_in_bin);

            // Rollback
            for (int b : cand_bins) {
                cur_dup_count_in_bin[b]++;
            }

            for (int b : cand_bins) {
                if (b != keep_bin) {
                    cur_bin_size[b]++;
                }
            }

            chosen_bin[idx] = -1;
        }
    };

    std::vector<int> cur_bin_size = bin_size;
    std::vector<int> cur_dup_count = dup_count_in_bin;

    dfs(0, cur_bin_size, cur_dup_count);

    // ---------------- Reconstruct best bins ----------------
    std::vector<std::vector<int>> best_bins = input_bins;

    for (int k = 0; k < D; k++) {
        int item = dup_items[k];
        int keep_bin = best_choice[k];

        for (int b : item_bins[item]) {
            if (b == keep_bin) continue;

            auto& bin = best_bins[b];
            auto it = std::find(bin.begin(), bin.end(), item);
            if (it != bin.end()) {
                bin.erase(it);
            }
        }
    }

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

bool LocalSearch::ejectionChain() {
    if (!sol || sol->bins.size() <= 1) return false;

    enum class NodeType { SOURCE, ITEM, ZERO };

    struct Pred {
        bool has_pred = false;
        NodeType type = NodeType::SOURCE;
        int id = -1;
    };

    struct NodeRef {
        NodeType type;
        int id;
    };

    struct Chain {
        double cost = 0.0;
        int zero_bin = -1;
        std::vector<NodeRef> path;   // source omitted, ends in ZERO
        std::unordered_set<int> items_used;
        std::unordered_set<int> bins_used;
    };

    const int n_bins = static_cast<int>(sol->bins.size());
    const int n_items = sol->N;
    const double INF = 1e18;

    auto penalty = [&](int load, int conflicts) -> int {
        int excess = std::max(0, load - sol->C);
        return K1 + K2 * excess + K3 * conflicts;
    };

    auto removalCost = [&](int item) -> int {
        int b = sol->item_bin[item];
        if (b < 0) return 0;

        int old_pen = penalty(sol->bin_loads[b], sol->bin_conflicts[b]);

        int new_load = sol->bin_loads[b] - sol->weights[item];
        int new_conf = sol->bin_conflicts[b] - sol->itemConflicts(item, b);
        int new_pen = penalty(new_load, new_conf);

        return new_pen - old_pen;
    };

    auto insertionCost = [&](int item, int bin) -> int {
        int old_pen = penalty(sol->bin_loads[bin], sol->bin_conflicts[bin]);

        int new_load = sol->bin_loads[bin] + sol->weights[item];
        int new_conf = sol->bin_conflicts[bin] + sol->itemConflicts(item, bin);
        int new_pen = penalty(new_load, new_conf);

        return new_pen - old_pen;
    };

    auto replacementCost = [&](int in_item, int out_item) -> int {
        int b = sol->item_bin[out_item];
        if (b < 0) return 0;

        int old_pen = penalty(sol->bin_loads[b], sol->bin_conflicts[b]);

        int conf_out = sol->itemConflicts(out_item, b);
        int conf_in = sol->itemConflicts(in_item, b);

        if (sol->hasConflict(in_item, out_item)) {
            conf_in -= 1;
        }

        int new_load = sol->bin_loads[b] - sol->weights[out_item] + sol->weights[in_item];
        int new_conf = sol->bin_conflicts[b] - conf_out + conf_in;
        int new_pen = penalty(new_load, new_conf);

        return new_pen - old_pen;
    };

    // -------------------- Random order Pi --------------------
    std::vector<int> pi(n_bins);
    std::iota(pi.begin(), pi.end(), 0);
    std::shuffle(pi.begin(), pi.end(), rng);

    std::vector<int> rank(n_bins, -1);
    for (int pos = 0; pos < n_bins; ++pos) {
        rank[pi[pos]] = pos;
    }

    std::vector<std::vector<int>> items_at_pos(n_bins);
    for (int item = 0; item < n_items; ++item) {
        int b = sol->item_bin[item];
        if (b >= 0) {
            items_at_pos[rank[b]].push_back(item);
        }
    }

    // -------------------- Shortest paths in DAG --------------------
    std::vector<double> dist_item(n_items, INF);
    std::vector<double> dist_zero(n_bins, INF);
    std::vector<Pred> pred_item(n_items);
    std::vector<Pred> pred_zero(n_bins);

    for (int item = 0; item < n_items; ++item) {
        dist_item[item] = removalCost(item);
        pred_item[item] = {true, NodeType::SOURCE, -1};
    }

    for (int pos = 0; pos < n_bins; ++pos) {
        int bin_from = pi[pos];

        std::vector<NodeRef> frontier;

        for (int item : items_at_pos[pos]) {
            if (dist_item[item] < INF) {
                frontier.push_back({NodeType::ITEM, item});
            }
        }

        if (dist_zero[bin_from] < INF) {
            frontier.push_back({NodeType::ZERO, bin_from});
        }

        if (frontier.empty()) continue;

        for (const NodeRef& u : frontier) {
            double du = (u.type == NodeType::ITEM ? dist_item[u.id] : dist_zero[u.id]);

            for (int next_pos = pos + 1; next_pos < n_bins; ++next_pos) {
                int bin_to = pi[next_pos];

                // to item nodes
                for (int out_item : items_at_pos[next_pos]) {
                    double arc = 0.0;

                    if (u.type == NodeType::ITEM) {
                        arc = replacementCost(u.id, out_item);
                    } else {
                        arc = removalCost(out_item);
                    }

                    if (du + arc < dist_item[out_item]) {
                        dist_item[out_item] = du + arc;
                        pred_item[out_item] = {true, u.type, u.id};
                    }
                }

                // to zero node
                if (u.type == NodeType::ITEM) {
                    double arc = insertionCost(u.id, bin_to);
                    if (du + arc < dist_zero[bin_to]) {
                        dist_zero[bin_to] = du + arc;
                        pred_zero[bin_to] = {true, NodeType::ITEM, u.id};
                    }
                }
            }
        }
    }

    // -------------------- Reconstruct all negative zero-node paths --------------------
    std::vector<Chain> candidate_chains;

    for (int zb = 0; zb < n_bins; ++zb) {
        if (dist_zero[zb] >= -1e-9) continue;

        std::vector<NodeRef> rev_path;
        NodeRef cur{NodeType::ZERO, zb};

        while (true) {
            rev_path.push_back(cur);

            if (cur.type == NodeType::ZERO) {
                Pred p = pred_zero[cur.id];
                if (!p.has_pred || p.type == NodeType::SOURCE) break;
                cur = {p.type, p.id};
            } else {
                Pred p = pred_item[cur.id];
                if (!p.has_pred || p.type == NodeType::SOURCE) break;
                cur = {p.type, p.id};
            }
        }

        std::reverse(rev_path.begin(), rev_path.end());

        Chain ch;
        ch.cost = dist_zero[zb];
        ch.zero_bin = zb;
        ch.path = rev_path;

        for (const NodeRef& node : rev_path) {
            if (node.type == NodeType::ITEM) {
                ch.items_used.insert(node.id);
                int b = sol->item_bin[node.id];
                if (b >= 0) ch.bins_used.insert(b);
            } else if (node.type == NodeType::ZERO) {
                ch.bins_used.insert(node.id);
            }
        }

        candidate_chains.push_back(std::move(ch));
    }

    if (candidate_chains.empty()) {
        return false;
    }

    std::sort(candidate_chains.begin(), candidate_chains.end(),
              [](const Chain& a, const Chain& b) {
                  return a.cost < b.cost;
              });

    // -------------------- Select disjoint chains greedily --------------------
    std::vector<Chain> selected;
    std::unordered_set<int> used_items;
    std::unordered_set<int> used_bins;

    for (const Chain& ch : candidate_chains) {
        bool disjoint = true;

        for (int item : ch.items_used) {
            if (used_items.count(item)) {
                disjoint = false;
                break;
            }
        }

        if (!disjoint) continue;

        for (int b : ch.bins_used) {
            if (used_bins.count(b)) {
                disjoint = false;
                break;
            }
        }

        if (!disjoint) continue;

        selected.push_back(ch);
        used_items.insert(ch.items_used.begin(), ch.items_used.end());
        used_bins.insert(ch.bins_used.begin(), ch.bins_used.end());
    }

    if (selected.empty()) {
        return false;
    }

    // -------------------- Apply all selected chains on a copy --------------------
    BPPCSolution candidate = *sol;

    for (const Chain& ch : selected) {
        int idx = 0;
        while (idx < (int)ch.path.size()) {
            if (ch.path[idx].type != NodeType::ITEM) {
                ++idx;
                continue;
            }

            int carried = ch.path[idx].id;
            int carried_from_bin = candidate.item_bin[carried];
            candidate.removeItemFromBin(carried, carried_from_bin);
            ++idx;

            while (idx < (int)ch.path.size()) {
                if (ch.path[idx].type == NodeType::ITEM) {
                    int out_item = ch.path[idx].id;
                    int b = candidate.item_bin[out_item];

                    candidate.removeItemFromBin(out_item, b);
                    candidate.addItemToBin(carried, b);

                    carried = out_item;
                    ++idx;
                } else if (ch.path[idx].type == NodeType::ZERO) {
                    int b = ch.path[idx].id;
                    candidate.addItemToBin(carried, b);
                    ++idx;
                    break;
                } else {
                    ++idx;
                    break;
                }
            }
        }
    }

    int old_obj = sol->computeObjective(K1, K2, K3);
    int new_obj = candidate.computeObjective(K1, K2, K3);

    if (new_obj < old_obj) {
        *sol = std::move(candidate);
        return true;
    }

    return false;
}

bool LocalSearch::grenade() {
    if (!sol || sol->bins.size() <= 1) return false;

    const int n_bins = static_cast<int>(sol->bins.size());
    const int old_obj = sol->computeObjective(K1, K2, K3);

    auto bestInsertionForItem = [&](const BPPCSolution& s, int item, int forbidden_bin,
                                    int& best_bin, int& best_delta) {
        best_bin = -1;
        best_delta = INT_MAX;

        int from_bin = s.item_bin[item];
        if (from_bin < 0) return false;

        for (int b = 0; b < (int)s.bins.size(); ++b) {
            if (b == forbidden_bin) continue;
            if (b == from_bin) continue;

            int delta = s.deltaMove(item, from_bin, b, K1, K2, K3);
            if (delta < best_delta) {
                best_delta = delta;
                best_bin = b;
            }
        }

        return best_bin != -1;
    };

    struct Relocation {
        int item = -1;
        int to_bin = -1;
    };

    // Problematic items = items belonging to bad bins
    std::vector<int> problematic;
    std::vector<char> used(sol->N, 0);

    for (int b : sol->bad_bins) {
        if (b < 0 || b >= n_bins) continue;

        for (int item : sol->bins[b]) {
            if (!used[item]) {
                used[item] = 1;
                problematic.push_back(item);
            }
        }
    }

    std::shuffle(problematic.begin(), problematic.end(), rng);

    for (int k : problematic) {
        int from_bin_k = sol->item_bin[k];
        if (from_bin_k < 0) continue;

        int best_total_delta = INT_MAX;
        int best_target_bin = -1;
        std::vector<Relocation> best_secondary_moves;

        for (int target_bin = 0; target_bin < n_bins; ++target_bin) {
            if (target_bin == from_bin_k) continue;

            BPPCSolution tmp = *sol;

            int delta_k = tmp.deltaMove(k, from_bin_k, target_bin, K1, K2, K3);
            tmp.moveItem(k, from_bin_k, target_bin);

            int total_delta = delta_k;
            std::vector<Relocation> secondary_moves;

            std::vector<int> conflicting_items;
            for (int item : tmp.bins[target_bin]) {
                if (item != k && tmp.hasConflict(k, item)) {
                    conflicting_items.push_back(item);
                }
            }

            std::shuffle(conflicting_items.begin(), conflicting_items.end(), rng);

            bool valid = true;

            for (int i : conflicting_items) {
                int best_bin_i = -1;
                int best_delta_i = INT_MAX;

                if (!bestInsertionForItem(tmp, i, target_bin, best_bin_i, best_delta_i)) {
                    valid = false;
                    break;
                }

                secondary_moves.push_back({i, best_bin_i});
                total_delta += best_delta_i;

                int cur_bin = tmp.item_bin[i];
                tmp.moveItem(i, cur_bin, best_bin_i);
            }

            if (!valid) continue;

            if (total_delta < best_total_delta) {
                best_total_delta = total_delta;
                best_target_bin = target_bin;
                best_secondary_moves = secondary_moves;
            }
        }

        if (best_target_bin != -1 && best_total_delta < 0) {
            BPPCSolution backup = *sol;

            int current_from_bin = sol->item_bin[k];
            sol->moveItem(k, current_from_bin, best_target_bin);

            for (const Relocation& mv : best_secondary_moves) {
                int cur_bin = sol->item_bin[mv.item];
                if (cur_bin >= 0 && cur_bin != mv.to_bin) {
                    sol->moveItem(mv.item, cur_bin, mv.to_bin);
                }
            }

            int new_obj = sol->computeObjective(K1, K2, K3);
            if (new_obj < old_obj) {
                return true;
            }

            *sol = std::move(backup);
        }
    }

    return false;
}

// Column generation: Without maximal column cuts
void LocalSearch::generateColumns() {
    bool verbose = false;

    int N_COLUMNS = 1;          // max columns per outer iteration
    int N_TIMES = 5;            // max number of outer iterations
    double RMP_TIME_LIMIT = 10.0;
    double PRICING_TIME_LIMIT = 5.0;
    const double RC_EPS = 1e-6;

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    auto statusToString = [](int status) -> std::string {
        switch (status) {
            case GRB_OPTIMAL:     return "OPTIMAL";
            case GRB_TIME_LIMIT:  return "TIME_LIMIT";
            case GRB_INFEASIBLE:  return "INFEASIBLE";
            case GRB_INTERRUPTED: return "INTERRUPTED";
            case GRB_UNBOUNDED:   return "UNBOUNDED";
            default:              return "OTHER(" + std::to_string(status) + ")";
        }
    };

    const int N = sol->N;
    size_t before = pool.size();

    if (pool.empty()) {
        log("[CG] empty pool");
        return;
    }

    auto total_start = std::chrono::high_resolution_clock::now();

    std::unordered_set<size_t> local_seen;
    int total_generated = 0;
    double total_rmp_time = 0.0;
    double total_pricing_time = 0.0;

    try {
        GRBEnv env(true);
        env.set("OutputFlag", "0");
        env.start();

        // Pricing model is reused across outer iterations
        GRBModel pricing(env);
        pricing.set(GRB_DoubleParam_TimeLimit, PRICING_TIME_LIMIT);
        pricing.set(GRB_IntParam_MIPFocus, 1);
        pricing.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

        std::vector<GRBVar> y(N);
        for (int i = 0; i < N; ++i) {
            y[i] = pricing.addVar(0.0, 1.0, 0.0, GRB_BINARY);
        }

        GRBLinExpr cap = 0;
        for (int i = 0; i < N; ++i) {
            cap += sol->weights[i] * y[i];
        }
        pricing.addConstr(cap <= sol->C, "capacity");

        for (int i = 0; i < N; ++i) {
            for (int j = i + 1; j < N; ++j) {
                if (sol->hasConflict(i, j)) {
                    pricing.addConstr(y[i] + y[j] <= 1,
                                      "conf_" + std::to_string(i) + "_" + std::to_string(j));
                }
            }
        }

        pricing.update();

        std::vector<GRBConstr> active_nogoods;

        for (int outer = 0; outer < N_TIMES; ++outer) {
            log("[CG] outer iteration " + std::to_string(outer + 1) +
                "/" + std::to_string(N_TIMES));

            std::vector<double> dual_values(N, 0.0);

            // Rebuild RMP on current pool
            {
                GRBModel rmp(env);
                rmp.set(GRB_DoubleParam_TimeLimit, RMP_TIME_LIMIT);

                int n_cols = static_cast<int>(pool.size());
                std::vector<GRBVar> x(n_cols);

                for (int j = 0; j < n_cols; ++j) {
                    x[j] = rmp.addVar(0.0, 1.0, pool[j].col.cost, GRB_CONTINUOUS);
                }

                std::vector<GRBConstr> cover(N);

                for (int i = 0; i < N; ++i) {
                    GRBLinExpr expr = 0;
                    for (int j = 0; j < n_cols; ++j) {
                        if (pool[j].col.mask[i]) expr += x[j];
                    }
                    cover[i] = rmp.addConstr(expr >= 1);
                }

                auto t0 = std::chrono::high_resolution_clock::now();
                rmp.optimize();
                auto t1 = std::chrono::high_resolution_clock::now();

                double rmp_time = std::chrono::duration<double>(t1 - t0).count();
                total_rmp_time += rmp_time;

                int status = rmp.get(GRB_IntAttr_Status);
                int sol_count = rmp.get(GRB_IntAttr_SolCount);
                bool optimal = (status == GRB_OPTIMAL);

                log("[CG] RMP status=" + statusToString(status) +
                    " optimal=" + std::string(optimal ? "yes" : "no") +
                    " sol_count=" + std::to_string(sol_count) +
                    " time=" + std::to_string(rmp_time) + " s" +
                    " n_cols=" + std::to_string(n_cols));

                if (status != GRB_OPTIMAL && status != GRB_TIME_LIMIT) {
                    log("[CG] stopping: RMP failed");
                    break;
                }

                if (sol_count == 0) {
                    log("[CG] stopping: RMP has no solution");
                    break;
                }

                for (int i = 0; i < N; ++i) {
                    dual_values[i] = cover[i].get(GRB_DoubleAttr_Pi);
                }
            }

            // Update pricing objective with fresh duals
            for (int i = 0; i < N; ++i) {
                y[i].set(GRB_DoubleAttr_Obj, dual_values[i]);
                y[i].set(GRB_DoubleAttr_Start, GRB_UNDEFINED);
            }

            // Remove no-good cuts from previous outer iteration
            if (!active_nogoods.empty()) {
                for (GRBConstr c : active_nogoods) {
                    pricing.remove(c);
                }
                active_nogoods.clear();
                pricing.update();
            }

            int generated_this_outer = 0;
            bool found_negative_rc = false;

            while (generated_this_outer < N_COLUMNS) {
                auto t0 = std::chrono::high_resolution_clock::now();
                pricing.optimize();
                auto t1 = std::chrono::high_resolution_clock::now();

                double mip_time = std::chrono::duration<double>(t1 - t0).count();
                total_pricing_time += mip_time;

                int status = pricing.get(GRB_IntAttr_Status);
                int sol_count = pricing.get(GRB_IntAttr_SolCount);
                bool optimal = (status == GRB_OPTIMAL);

                log("[CG] Pricing status=" + statusToString(status) +
                    " optimal=" + std::string(optimal ? "yes" : "no") +
                    " sol_count=" + std::to_string(sol_count) +
                    " time=" + std::to_string(mip_time) + " s" +
                    " outer=" + std::to_string(outer + 1) +
                    " generated_in_outer=" + std::to_string(generated_this_outer));

                if ((status != GRB_OPTIMAL && status != GRB_TIME_LIMIT) || sol_count == 0) {
                    log("[CG] stopping pricing: no pricing solution");
                    break;
                }

                std::vector<int> items;
                items.reserve(N);

                double dual_sum = 0.0;
                for (int i = 0; i < N; ++i) {
                    if (y[i].get(GRB_DoubleAttr_X) > 0.5) {
                        items.push_back(i);
                        dual_sum += dual_values[i];
                    }
                }

                if (items.empty()) {
                    log("[CG] stopping pricing: empty column");
                    break;
                }

                std::sort(items.begin(), items.end());
                size_t h = hashBin(items);

                double rc = K1 - dual_sum;

                log("[CG] column size=" + std::to_string(items.size()) +
                    " dual_sum=" + std::to_string(dual_sum) +
                    " rc=" + std::to_string(rc) +
                    " proven_by_optimality=" + std::string(optimal ? "yes" : "no"));

                if (rc >= -RC_EPS) {
                    log("[CG] stopping pricing: no negative reduced-cost column");
                    break;
                }

                found_negative_rc = true;

                if (!local_seen.count(h)) {
                    local_seen.insert(h);

                    Column col;
                    col.items = items;
                    col.buildMask(N);
                    col.cost = K1;

                    addColumn(std::move(col), N);
                    ++generated_this_outer;
                    ++total_generated;

                    log("[CG] added column, total_generated=" + std::to_string(total_generated));
                } else {
                    log("[CG] duplicate column skipped");
                }

                GRBLinExpr nogood = 0;
                std::vector<char> in_col(N, false);
                for (int i : items) in_col[i] = true;

                for (int i = 0; i < N; ++i) {
                    if (in_col[i]) nogood += (1 - y[i]);
                    else           nogood += y[i];
                }

                GRBConstr cut = pricing.addConstr(nogood >= 1);
                active_nogoods.push_back(cut);
                pricing.update();
            }

            log("[CG] outer iteration result: generated_this_outer=" +
                std::to_string(generated_this_outer));

            if (!found_negative_rc) {
                log("[CG] stopping outer loop: no negative reduced-cost column found");
                break;
            }

            if (generated_this_outer == 0) {
                log("[CG] stopping outer loop: no new columns added");
                break;
            }
        }

    } catch (GRBException& e) {
        log("[CG] Gurobi error: " + std::string(e.getMessage()));
        return;
    } catch (...) {
        log("[CG] unknown error");
        return;
    }

    auto total_end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(total_end - total_start).count();

    log("[CG] total_generated=" + std::to_string(total_generated));
    log("[CG] pool_added=" + std::to_string(pool.size() - before));
    log("[CG] total_rmp_time=" + std::to_string(total_rmp_time) + " s");
    log("[CG] total_pricing_time=" + std::to_string(total_pricing_time) + " s");
    log("[CG] total_function_time=" + std::to_string(total_time) + " s");
    log("[CG] pool_after=" + std::to_string(pool.size()));

    trimPool();
}

// Column generation: With maximal column cuts
// void LocalSearch::generateColumns() {
//     bool verbose = true;

//     int N_COLUMNS = 1;
//     int N_TIMES = 10;
//     double RMP_TIME_LIMIT = 10.0;
//     double PRICING_TIME_LIMIT = 5.0;
//     const double RC_EPS = 1e-6;

//     auto log = [&](const std::string& msg) {
//         if (verbose) std::cout << msg << "\n";
//     };

//     auto statusToString = [](int status) -> std::string {
//         switch (status) {
//             case GRB_OPTIMAL:     return "OPTIMAL";
//             case GRB_TIME_LIMIT:  return "TIME_LIMIT";
//             case GRB_INFEASIBLE:  return "INFEASIBLE";
//             case GRB_INTERRUPTED: return "INTERRUPTED";
//             case GRB_UNBOUNDED:   return "UNBOUNDED";
//             default:              return "OTHER(" + std::to_string(status) + ")";
//         }
//     };

//     const int N = sol->N;
//     size_t before = pool.size();

//     if (pool.empty()) {
//         log("[CG] empty pool");
//         return;
//     }

//     auto total_start = std::chrono::high_resolution_clock::now();

//     std::unordered_set<size_t> local_seen;
//     int total_generated = 0;
//     double total_rmp_time = 0.0;
//     double total_pricing_time = 0.0;

//     try {
//         GRBEnv env(true);
//         env.set("OutputFlag", "0");
//         env.start();

//         for (int outer = 0; outer < N_TIMES; ++outer) {
//             log("[CG] outer iteration " + std::to_string(outer + 1) +
//                 "/" + std::to_string(N_TIMES));

//             std::vector<double> dual_values(N, 0.0);

//             {
//                 GRBModel rmp(env);
//                 rmp.set(GRB_DoubleParam_TimeLimit, RMP_TIME_LIMIT);

//                 int n_cols = static_cast<int>(pool.size());
//                 std::vector<GRBVar> x(n_cols);

//                 for (int j = 0; j < n_cols; ++j) {
//                     x[j] = rmp.addVar(0.0, 1.0, pool[j].col.cost, GRB_CONTINUOUS);
//                 }

//                 std::vector<GRBConstr> cover(N);

//                 for (int i = 0; i < N; ++i) {
//                     GRBLinExpr expr = 0;
//                     for (int j = 0; j < n_cols; ++j) {
//                         if (pool[j].col.mask[i]) expr += x[j];
//                     }
//                     cover[i] = rmp.addConstr(expr >= 1);
//                 }

//                 auto t0 = std::chrono::high_resolution_clock::now();
//                 rmp.optimize();
//                 auto t1 = std::chrono::high_resolution_clock::now();

//                 double rmp_time = std::chrono::duration<double>(t1 - t0).count();
//                 total_rmp_time += rmp_time;

//                 int status = rmp.get(GRB_IntAttr_Status);
//                 int sol_count = rmp.get(GRB_IntAttr_SolCount);
//                 bool optimal = (status == GRB_OPTIMAL);

//                 log("[CG] RMP status=" + statusToString(status) +
//                     " optimal=" + std::string(optimal ? "yes" : "no") +
//                     " sol_count=" + std::to_string(sol_count) +
//                     " time=" + std::to_string(rmp_time) + " s" +
//                     " n_cols=" + std::to_string(n_cols));

//                 if (status != GRB_OPTIMAL && status != GRB_TIME_LIMIT) {
//                     log("[CG] stopping: RMP failed");
//                     break;
//                 }

//                 if (sol_count == 0) {
//                     log("[CG] stopping: RMP has no solution");
//                     break;
//                 }

//                 for (int i = 0; i < N; ++i) {
//                     dual_values[i] = cover[i].get(GRB_DoubleAttr_Pi);
//                 }
//             }

//             GRBModel pricing(env);
//             pricing.set(GRB_DoubleParam_TimeLimit, PRICING_TIME_LIMIT);
//             pricing.set(GRB_IntParam_MIPFocus, 1);
//             pricing.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

//             std::vector<GRBVar> y(N);
//             for (int i = 0; i < N; ++i) {
//                 y[i] = pricing.addVar(0.0, 1.0, dual_values[i], GRB_BINARY);
//             }

//             GRBLinExpr cap = 0;
//             for (int i = 0; i < N; ++i) {
//                 cap += sol->weights[i] * y[i];
//             }
//             pricing.addConstr(cap <= sol->C, "capacity");

//             for (int i = 0; i < N; ++i) {
//                 for (int j = i + 1; j < N; ++j) {
//                     if (sol->hasConflict(i, j)) {
//                         pricing.addConstr(y[i] + y[j] <= 1,
//                                           "conf_" + std::to_string(i) + "_" + std::to_string(j));
//                     }
//                 }
//             }

//             int subset_cut_count = 0;
//             for (int c = 0; c < static_cast<int>(pool.size()); ++c) {
//                 GRBLinExpr forbid_subset = 0;

//                 for (int i = 0; i < N; ++i) {
//                     if (!pool[c].col.mask[i]) {
//                         forbid_subset += y[i];
//                     }
//                 }

//                 pricing.addConstr(
//                     forbid_subset >= 1,
//                     "nonsubset_" + std::to_string(c)
//                 );
//                 ++subset_cut_count;
//             }

//             pricing.update();

//             log("[CG] pricing subset cuts added=" + std::to_string(subset_cut_count));

//             int generated_this_outer = 0;
//             bool found_negative_rc = false;
//             std::vector<GRBConstr> active_nogoods;

//             while (generated_this_outer < N_COLUMNS) {
//                 auto t0 = std::chrono::high_resolution_clock::now();
//                 pricing.optimize();
//                 auto t1 = std::chrono::high_resolution_clock::now();

//                 double mip_time = std::chrono::duration<double>(t1 - t0).count();
//                 total_pricing_time += mip_time;

//                 int status = pricing.get(GRB_IntAttr_Status);
//                 int sol_count = pricing.get(GRB_IntAttr_SolCount);
//                 bool optimal = (status == GRB_OPTIMAL);

//                 log("[CG] Pricing status=" + statusToString(status) +
//                     " optimal=" + std::string(optimal ? "yes" : "no") +
//                     " sol_count=" + std::to_string(sol_count) +
//                     " time=" + std::to_string(mip_time) + " s" +
//                     " outer=" + std::to_string(outer + 1) +
//                     " generated_in_outer=" + std::to_string(generated_this_outer));

//                 if ((status != GRB_OPTIMAL && status != GRB_TIME_LIMIT) || sol_count == 0) {
//                     log("[CG] stopping pricing: no pricing solution");
//                     break;
//                 }

//                 std::vector<int> items;
//                 items.reserve(N);

//                 double dual_sum = 0.0;
//                 for (int i = 0; i < N; ++i) {
//                     if (y[i].get(GRB_DoubleAttr_X) > 0.5) {
//                         items.push_back(i);
//                         dual_sum += dual_values[i];
//                     }
//                 }

//                 if (items.empty()) {
//                     log("[CG] stopping pricing: empty column");
//                     break;
//                 }

//                 std::sort(items.begin(), items.end());
//                 size_t h = hashBin(items);

//                 double rc = K1 - dual_sum;

//                 log("[CG] column size=" + std::to_string(items.size()) +
//                     " dual_sum=" + std::to_string(dual_sum) +
//                     " rc=" + std::to_string(rc) +
//                     " proven_by_optimality=" + std::string(optimal ? "yes" : "no"));

//                 if (rc >= -RC_EPS) {
//                     log("[CG] stopping pricing: no negative reduced-cost column");
//                     break;
//                 }

//                 found_negative_rc = true;

//                 if (!local_seen.count(h)) {
//                     local_seen.insert(h);

//                     Column col;
//                     col.items = items;
//                     col.buildMask(N);
//                     col.cost = K1;

//                     addColumn(std::move(col), N);
//                     ++generated_this_outer;
//                     ++total_generated;

//                     log("[CG] added column, total_generated=" + std::to_string(total_generated));
//                 } else {
//                     log("[CG] duplicate column skipped");
//                 }

//                 GRBLinExpr nogood = 0;
//                 std::vector<char> in_col(N, false);
//                 for (int i : items) in_col[i] = true;

//                 for (int i = 0; i < N; ++i) {
//                     if (in_col[i]) nogood += (1 - y[i]);
//                     else           nogood += y[i];
//                 }

//                 GRBConstr cut = pricing.addConstr(nogood >= 1);
//                 active_nogoods.push_back(cut);
//                 pricing.update();
//             }

//             log("[CG] outer iteration result: generated_this_outer=" +
//                 std::to_string(generated_this_outer));

//             if (!found_negative_rc) {
//                 log("[CG] stopping outer loop: no negative reduced-cost column found");
//                 break;
//             }

//             if (generated_this_outer == 0) {
//                 log("[CG] stopping outer loop: no new columns added");
//                 break;
//             }
//         }

//     } catch (GRBException& e) {
//         log("[CG] Gurobi error: " + std::string(e.getMessage()));
//         return;
//     } catch (...) {
//         log("[CG] unknown error");
//         return;
//     }

//     auto total_end = std::chrono::high_resolution_clock::now();
//     double total_time = std::chrono::duration<double>(total_end - total_start).count();

//     log("[CG] total_generated=" + std::to_string(total_generated));
//     log("[CG] pool_added=" + std::to_string(pool.size() - before));
//     log("[CG] total_rmp_time=" + std::to_string(total_rmp_time) + " s");
//     log("[CG] total_pricing_time=" + std::to_string(total_pricing_time) + " s");
//     log("[CG] total_function_time=" + std::to_string(total_time) + " s");
//     log("[CG] pool_after=" + std::to_string(pool.size()));

//     trimPool();
// }
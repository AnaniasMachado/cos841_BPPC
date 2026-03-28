#include "local_search.hpp"
#include <algorithm>
#include <random>
#include <iostream>

LocalSearchRVND::LocalSearchRVND(BPPCSolution& solution, int k_bins, int k_excess, int k_conflicts)
    : sol(solution), K1(k_bins), K2(k_excess), K3(k_conflicts) {}

// -------------------- Main RVND --------------------
void LocalSearchRVND::run() {
    std::vector<int> neighborhoods = {0, 1, 2}; // 0=relocation, 1=exchange, 2=add
    std::random_device rd;
    std::mt19937 rng(rd());

    while (!neighborhoods.empty()) {
        std::uniform_int_distribution<int> dist(0, neighborhoods.size() - 1);
        int idx = dist(rng);
        int n = neighborhoods[idx];

        bool improved = false;
        switch(n) {
            case 0: improved = relocation(); break;
            case 1: improved = exchange(); break;
            case 2: improved = add(); break;
        }

        if (improved) {
            neighborhoods = {0, 1, 2};
        } else {
            neighborhoods.erase(neighborhoods.begin() + idx);
        }
    }
}

// -------------------- Relocation --------------------
bool LocalSearchRVND::relocation() {
    int best_obj = computeObjective(sol);
    int best_from = -1, best_to = -1, best_item = -1;

    for (size_t from = 0; from < sol.bins.size(); from++) {
        for (size_t i = 0; i < sol.bins[from].size(); i++) {
            int item = sol.bins[from][i];

            for (size_t to = 0; to < sol.bins.size(); to++) {
                if (to == from) continue;

                BPPCSolution temp = sol;
                temp.moveItem(item, from, to);

                int obj = computeObjective(temp);

                // STRICT improvement (safe)
                if (obj < best_obj) {
                    best_obj = obj;
                    best_from = from;
                    best_to = to;
                    best_item = item;
                }
            }
        }
    }

    if (best_item != -1) {
        sol.moveItem(best_item, best_from, best_to);
        sol.removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Exchange --------------------
bool LocalSearchRVND::exchange() {
    int best_obj = computeObjective(sol);
    int best_from1=-1, best_from2=-1, best_i=-1, best_j=-1;

    for (size_t b1 = 0; b1 < sol.bins.size(); b1++) {
        for (size_t i = 0; i < sol.bins[b1].size(); i++) {

            for (size_t b2 = b1+1; b2 < sol.bins.size(); b2++) {
                for (size_t j = 0; j < sol.bins[b2].size(); j++) {

                    BPPCSolution temp = sol;
                    temp.swapItems(b1, i, b2, j);

                    int obj = computeObjective(temp);

                    if (obj < best_obj) {
                        best_obj = obj;
                        best_from1 = b1;
                        best_from2 = b2;
                        best_i = i;
                        best_j = j;
                    }
                }
            }
        }
    }

    if (best_i != -1) {
        sol.swapItems(best_from1, best_i, best_from2, best_j);
        sol.removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Add --------------------
bool LocalSearchRVND::add() {
    int best_obj = computeObjective(sol);
    int best_from=-1, best_item=-1;

    for (size_t b = 0; b < sol.bins.size(); b++) {
        for (size_t i = 0; i < sol.bins[b].size(); i++) {
            int item = sol.bins[b][i];

            BPPCSolution temp = sol;
            temp.moveItem(item, b, sol.bins.size());

            int obj = computeObjective(temp);

            if (obj < best_obj) {
                best_obj = obj;
                best_from = b;
                best_item = item;
            }
        }
    }

    if (best_item != -1) {
        sol.moveItem(best_item, best_from, sol.bins.size());
        sol.removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Objective computation --------------------
int LocalSearchRVND::computeObjective(const BPPCSolution& s) const {
    return s.computeObjective(K1, K2, K3);
}

// -------------------- Return solution --------------------
BPPCSolution LocalSearchRVND::getSolution() const {
    return sol;
}
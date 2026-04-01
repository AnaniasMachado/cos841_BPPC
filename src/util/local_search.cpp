#include "local_search.hpp"
#include <algorithm>

// -------------------- Constructor --------------------
LocalSearch::LocalSearch(BPPCSolution& solution, int k1, int k2, int k3)
    : sol(&solution), K1(k1), K2(k2), K3(k3) {}

// -------------------- Set Solution --------------------
void LocalSearch::setSolution(BPPCSolution& solution) {
    sol = &solution;
}

// -------------------- Relocation (delta version) --------------------
bool LocalSearch::relocation() {
    int current_obj = computeObjective(*sol);
    int best_delta = 0;

    int best_from = -1, best_to = -1, best_item = -1;

    for (size_t from = 0; from < sol->bins.size(); from++) {
        for (size_t i = 0; i < sol->bins[from].size(); i++) {
            int item = sol->bins[from][i];

            for (size_t to = 0; to <= sol->bins.size(); to++) {
                if (to == from) continue;

                int delta = sol->deltaMove(item, from, to, K1, K2, K3);

                if (delta < best_delta) {
                    best_delta = delta;
                    best_from = from;
                    best_to = to;
                    best_item = item;
                }
            }
        }
    }

    if (best_item != -1) {
        sol->moveItem(best_item, best_from, best_to);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Exchange (delta version) --------------------
bool LocalSearch::exchange() {
    int best_delta = 0;

    int b1_best = -1, b2_best = -1;
    int i_best = -1, j_best = -1;

    for (size_t b1 = 0; b1 < sol->bins.size(); b1++) {
        for (size_t i = 0; i < sol->bins[b1].size(); i++) {

            for (size_t b2 = b1 + 1; b2 < sol->bins.size(); b2++) {
                for (size_t j = 0; j < sol->bins[b2].size(); j++) {

                    int delta = sol->deltaSwap(b1, i, b2, j, K1, K2, K3);

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

    if (i_best != -1) {
        sol->swapItems(b1_best, i_best, b2_best, j_best);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Add (delta version) --------------------
bool LocalSearch::add() {
    int best_delta = 0;

    int best_from = -1, best_item = -1;

    size_t new_bin = sol->bins.size();

    for (size_t b = 0; b < sol->bins.size(); b++) {
        for (size_t i = 0; i < sol->bins[b].size(); i++) {
            int item = sol->bins[b][i];

            int delta = sol->deltaMove(item, b, new_bin, K1, K2, K3);

            if (delta < best_delta) {
                best_delta = delta;
                best_from = b;
                best_item = item;
            }
        }
    }

    if (best_item != -1) {
        sol->moveItem(best_item, best_from, new_bin);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Objective --------------------
int LocalSearch::computeObjective(const BPPCSolution& s) const {
    return s.computeObjective(K1, K2, K3);
}
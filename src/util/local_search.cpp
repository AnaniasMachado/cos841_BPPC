#include "local_search.hpp"
#include <algorithm>

// -------------------- Constructor --------------------
LocalSearch::LocalSearch(BPPCSolution& solution, int k1, int k2, int k3)
    : sol(&solution), K1(k1), K2(k2), K3(k3) {}

// -------------------- Set Solution (NEW CLEAN DESIGN) --------------------
void LocalSearch::setSolution(BPPCSolution& solution) {
    sol = &solution;
}

// -------------------- Relocation --------------------
bool LocalSearch::relocation() {
    int best_obj = computeObjective(*sol);
    int best_from=-1, best_to=-1, best_item=-1;

    for (size_t from = 0; from < sol->bins.size(); from++) {
        for (size_t i = 0; i < sol->bins[from].size(); i++) {
            int item = sol->bins[from][i];

            for (size_t to = 0; to <= sol->bins.size(); to++) {
                if (to == from) continue;

                BPPCSolution temp = *sol;
                temp.moveItem(item, from, to);

                int obj = computeObjective(temp);

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
        sol->moveItem(best_item, best_from, best_to);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Exchange --------------------
bool LocalSearch::exchange() {
    int best_obj = computeObjective(*sol);
    int b1_best=-1, b2_best=-1, i_best=-1, j_best=-1;

    for (size_t b1 = 0; b1 < sol->bins.size(); b1++) {
        for (size_t i = 0; i < sol->bins[b1].size(); i++) {
            for (size_t b2 = b1+1; b2 < sol->bins.size(); b2++) {
                for (size_t j = 0; j < sol->bins[b2].size(); j++) {

                    BPPCSolution temp = *sol;
                    temp.swapItems(b1,i,b2,j);

                    int obj = computeObjective(temp);

                    if (obj < best_obj) {
                        best_obj = obj;
                        b1_best=b1; b2_best=b2;
                        i_best=i; j_best=j;
                    }
                }
            }
        }
    }

    if (i_best != -1) {
        sol->swapItems(b1_best,i_best,b2_best,j_best);
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Add --------------------
bool LocalSearch::add() {
    int best_obj = computeObjective(*sol);
    int best_from=-1, best_item=-1;

    for (size_t b = 0; b < sol->bins.size(); b++) {
        for (size_t i = 0; i < sol->bins[b].size(); i++) {
            int item = sol->bins[b][i];

            BPPCSolution temp = *sol;
            temp.moveItem(item, b, sol->bins.size());

            int obj = computeObjective(temp);

            if (obj < best_obj) {
                best_obj = obj;
                best_from = b;
                best_item = item;
            }
        }
    }

    if (best_item != -1) {
        sol->moveItem(best_item, best_from, sol->bins.size());
        sol->removeEmptyBins();
        return true;
    }

    return false;
}

// -------------------- Objective --------------------
int LocalSearch::computeObjective(const BPPCSolution& s) const {
    return s.computeObjective(K1, K2, K3);
}
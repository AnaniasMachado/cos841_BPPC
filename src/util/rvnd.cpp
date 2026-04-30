#include "rvnd.hpp"

// -------------------- Constructor --------------------
RVND::RVND(BPPCSolution& solution, ImprovementType improvement_type_,
            int k1, int k2, int k3)
    : sol(&solution),
      ls(*sol, improvement_type_, k1, k2, k3),
      K1(k1), K2(k2), K3(k3)
{
    std::random_device rd;
    rng = std::mt19937(rd());
    iter = -1;
}

// -------------------- RVND --------------------
void RVND::run() {

    // Initial random permutation p[r]
    std::vector<int> p = {0, 1, 2, 3};
    std::shuffle(p.begin(), p.end(), rng);

    int k = 0;
    bool improved_global = false;


    // RVND loop
    while (k < (int)p.size()) {

        bool improved = false;

        switch (p[k]) {
            case 0: improved = ls.classic();                    break;
            case 1: improved = ls.assignment((int)sol->N / 5);  break;
            case 2: improved = ls.ejectionChain();              break;
            case 3: improved = ls.grenade();                    break;
        }

        if (improved) {
            improved_global = true;

            // Restart RVND
            k = 0;
        } else {
            k++;
        }
    }

    // if (iter % 25 == 0 && iter >= 25) {
    //     ls.generateColumns();
    //     bool sc_improve = ls.setCovering();
    //     if (sc_improve) improved_global = true;
    // }

    if (improved_global) {
        ls.updateElite();
        ls.updateK();
        ls.updatePool();
    }

    iter++;
}

void RVND::setSolution(BPPCSolution& solution) {
    sol = &solution;
    ls.setSolution(*sol);
}
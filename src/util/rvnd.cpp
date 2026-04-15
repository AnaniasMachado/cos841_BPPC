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

        bool local_improved = false;

        switch (p[k]) {
            case 0: local_improved = ls.classic();                      break;
            case 2: local_improved = ls.ejectionGlobal();               break;
            case 3: local_improved = ls.assignment((int)sol->N / 5);    break;
        }

        if (local_improved) {
            improved_global = true;

            // Restart RVND
            k = 0;
        } else {
            k++;
        }
    }

    if (iter % 5 == 0 && iter >= 15) {
        // bool sc_improve = true;
        // while (sc_improve) {
        //     sc_improve = ls.setCovering();
        //     if (sc_improve) {
        //         ls.updateK();
        //         ls.updateElite(*sol);
        //     }
        // }
        bool sc_improve = ls.setCovering();
        if (sc_improve) improved_global = true;
    }

    if (improved_global) {
        ls.updateK();
        ls.updateElite(*sol);
        ls.updatePool();
    }

    iter++;
}

void RVND::setSolution(BPPCSolution& solution) {
    sol = &solution;
    ls.setSolution(*sol);
}
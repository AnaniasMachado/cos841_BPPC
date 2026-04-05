#include "rvnd.hpp"

// -------------------- Constructor --------------------
RVND::RVND(BPPCSolution& solution, int k1, int k2, int k3)
    : sol(&solution),
      ls(*sol, k1, k2, k3),
      K1(k1), K2(k2), K3(k3)
{
    std::random_device rd;
    rng = std::mt19937(rd());
}

// -------------------- RVND --------------------
void RVND::run() {
    // Shuffle permutation/order of neighborhoods
    std::vector<int> neighborhoods = {0, 1, 2};
    std::shuffle(neighborhoods.begin(), neighborhoods.end(), rng);

    bool improved = true;
    
    while (improved) {
        improved = false;

        for (int n : neighborhoods) {
            bool local_improved = false;
            
            switch(n) {
                case 0: local_improved = ls.relocation(); break;
                case 1: local_improved = ls.exchange();   break;
                case 2: local_improved = ls.ejection();   break;
            }

            if (local_improved) {
                improved = true;
            }
        }
    }
}

void RVND::setSolution(BPPCSolution& solution) {
    sol = &solution;
    ls.setSolution(*sol);
}
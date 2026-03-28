#include "rvnd.hpp"

// -------------------- Constructor --------------------
RVND::RVND(BPPCSolution& solution, int k1, int k2, int k3)
    : sol(solution), ls(sol, k1, k2, k3)
{
    std::random_device rd;
    rng = std::mt19937(rd());
}

// -------------------- RVND --------------------
void RVND::run() {
    std::vector<int> neighborhoods = {0,1,2};

    while (!neighborhoods.empty()) {
        std::uniform_int_distribution<int> dist(0, neighborhoods.size()-1);
        int idx = dist(rng);
        int n = neighborhoods[idx];

        bool improved = false;

        switch(n) {
            case 0: improved = ls.relocation(); break;
            case 1: improved = ls.exchange(); break;
            case 2: improved = ls.add(); break;
        }

        if (improved) {
            neighborhoods = {0,1,2};
        } else {
            neighborhoods.erase(neighborhoods.begin()+idx);
        }
    }
}
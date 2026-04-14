#include "qrvnd.hpp"
#include <algorithm>

// -------------------- Constructor --------------------
QRVND::QRVND(BPPCSolution& solution, ImprovementType improvement_type_,
             int k1, int k2, int k3,
             double a, double g, double e)
    : sol(&solution),
      ls(*sol, improvement_type_, k1, k2, k3),
      K1(k1), K2(k2), K3(k3),
      alpha(a), gamma(g), epsilon(e),
      dist01(0.0, 1.0)
{
    std::random_device rd;
    rng = std::mt19937(rd());
    iter = -1;

    initialized = false;
    current_p = 0;

    // perms = {
    //     {0,1,2},{0,2,1},
    //     {1,0,2},{1,2,0},
    //     {2,0,1},{2,1,0}
    // };

    perms = {
        {0,1,2,3}, {0,1,3,2}, {0,2,1,3}, {0,2,3,1}, {0,3,1,2}, {0,3,2,1},
        {1,0,2,3}, {1,0,3,2}, {1,2,0,3}, {1,2,3,0}, {1,3,0,2}, {1,3,2,0},
        {2,0,1,3}, {2,0,3,1}, {2,1,0,3}, {2,1,3,0}, {2,3,0,1}, {2,3,1,0},
        {3,0,1,2}, {3,0,2,1}, {3,1,0,2}, {3,1,2,0}, {3,2,0,1}, {3,2,1,0}
    };

    Q = std::vector<std::vector<double>>(
        perms.size(),
        std::vector<double>(perms.size(), 0.0)
    );
}

// -------------------- Select --------------------
int QRVND::selectPermutation(int current_p) {

    // Exploration
    if (dist01(rng) < epsilon) {
        std::uniform_int_distribution<int> d(0, perms.size()-1);
        return d(rng);
    }

    // Exploitation
    return std::distance(
        Q[current_p].begin(),
        std::max_element(Q[current_p].begin(), Q[current_p].end())
    );
}

// -------------------- Apply ONE RVND --------------------
bool QRVND::applyOrder(const std::vector<int>& order) {

    bool improved_global = false;

    std::vector<int> c = order;  // Permutation p[r]
    int k = 0;

    while (k < (int)c.size()) {

        bool improved = false;

        switch (c[k]) {
            case 0: improved = ls.relocation();      break;
            case 1: improved = ls.exchange();        break;
            case 2: improved = ls.ejectionGlobal();  break;
            case 3: improved = ls.assignment();      break;
        }

        if (improved) {
            improved_global = true;
            // ls.updatePool();
            // BPPCSolution perturbed = ls.destroyRepair();
            // ls.addToPool(perturbed);

            // Restart RVND
            k = 0;
        } else {
            k++;
        }
    }

    if (iter % 5 == 0 && iter != 0) {
        bool sc_improve = true;
        while (sc_improve) {
            sc_improve = ls.setCovering();
            if (sc_improve) {
                ls.updateK();
                ls.updateElite(*sol);
            }
        }
    }

    if (improved_global) {
        ls.updateK();
        ls.updateElite(*sol);
        ls.updatePool();
    }

    iter++;

    return improved_global;
}

// -------------------- Run (ONE iteration only) --------------------
void QRVND::run() {

    if (!initialized) {
        std::uniform_int_distribution<int> d(0, perms.size()-1);
        current_p = d(rng);
        initialized = true;
    }

    int next_p = selectPermutation(current_p);

    int before = sol->computeObjective(K1, K2, K3);

    // ONE RVND only
    bool improved = applyOrder(perms[next_p]);

    int after = sol->computeObjective(K1, K2, K3);

    // Reward
    double reward = (before > 0)
        ? (before - after) / (double) before
        : 0.0;

    // Q-learning update
    double max_next = *std::max_element(Q[next_p].begin(), Q[next_p].end());

    Q[current_p][next_p] += alpha *
        (reward + gamma * max_next - Q[current_p][next_p]);

    // Move state
    current_p = next_p;

    // Mild epsilon decay (optional)
    epsilon = std::max(0.05, epsilon * 0.9);
}

void QRVND::setSolution(BPPCSolution& solution) {
    sol = &solution;
    ls.setSolution(*sol);
}
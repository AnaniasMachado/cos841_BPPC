#include "qrvnd.hpp"
#include <algorithm>

// -------------------- Constructor --------------------
QRVND::QRVND(BPPCSolution& solution, int k1, int k2, int k3,
             double a, double g, double e)
    : sol(&solution),
      ls(*sol, k1, k2, k3),
      K1(k1), K2(k2), K3(k3),
      alpha(a), gamma(g), epsilon(e),
      dist01(0.0, 1.0)
{
    std::random_device rd;
    rng = std::mt19937(rd());

    initialized = false;
    current_p = 0;

    perms = {
        {0,1,2},{0,2,1},
        {1,0,2},{1,2,0},
        {2,0,1},{2,1,0}
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

    std::vector<int> remaining = order;

    while (!remaining.empty()) {

        int n = remaining.front();
        remaining.erase(remaining.begin());

        bool improved = false;

        switch(n) {
            case 0: improved = ls.relocation(); break;
            case 1: improved = ls.exchange();   break;
            case 2: improved = ls.ejection();   break;
        }

        if (improved) {
            improved_global = true;

            // Reset to full order
            remaining = order;
        }
    }

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
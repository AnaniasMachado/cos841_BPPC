#include "qrvnd.hpp"
#include <algorithm>

// -------------------- Constructor --------------------
QRVND::QRVND(BPPCSolution& solution, int k1, int k2, int k3,
             double a, double g, double e,
             int max_it, int max_no_imp)
    : sol(solution),
      ls(sol, k1, k2, k3),
      K1(k1), K2(k2), K3(k3),
      alpha(a), gamma(g), epsilon(e),
      max_iterations(max_it), max_no_improve(max_no_imp)
{
    std::random_device rd;
    rng = std::mt19937(rd());

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
    std::uniform_real_distribution<double> dist(0.0,1.0);

    if (dist(rng) < epsilon) {
        std::uniform_int_distribution<int> d(0, perms.size()-1);
        return d(rng);
    }

    return std::distance(
        Q[current_p].begin(),
        std::max_element(Q[current_p].begin(), Q[current_p].end())
    );
}

// -------------------- Apply --------------------
bool QRVND::applyOrder(const std::vector<int>& order) {
    bool improved_global = false;
    std::vector<int> remaining = order;

    while (!remaining.empty()) {
        std::uniform_int_distribution<int> dist(0, remaining.size()-1);
        int idx = dist(rng);
        int n = remaining[idx];

        bool improved = false;

        switch(n) {
            case 0: improved = ls.relocation(); break;
            case 1: improved = ls.exchange(); break;
            case 2: improved = ls.add(); break;
        }

        if (improved) {
            improved_global = true;
            remaining = order;
        } else {
            remaining.erase(remaining.begin()+idx);
        }
    }

    return improved_global;
}

// -------------------- Run --------------------
void QRVND::run() {
    int iter = 0, no_improve = 0;

    int current_p = std::uniform_int_distribution<int>(0, perms.size()-1)(rng);

    while (iter < max_iterations && no_improve < max_no_improve) {

        int next_p = selectPermutation(current_p);

        int before = sol.computeObjective(K1,K2,K3);

        bool improved = applyOrder(perms[next_p]);

        int after = sol.computeObjective(K1,K2,K3);

        double reward = (before > 0) ? (before - after)/(double)before : 0.0;

        double max_next = *std::max_element(Q[next_p].begin(), Q[next_p].end());

        Q[current_p][next_p] += alpha *
            (reward + gamma * max_next - Q[current_p][next_p]);

        current_p = next_p;

        if (improved) no_improve = 0;
        else no_improve++;

        epsilon = std::max(0.05, epsilon * 0.995);

        iter++;
    }
}
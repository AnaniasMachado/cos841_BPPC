#include "ails.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>

// -------------------- Constructor --------------------
AILS::AILS(const BPPCInstance& instance,
           int k1, int k2, int k3,
           int max_it,
           int max_no_imp)
    : inst(instance),
      K1(k1), K2(k2), K3(k3),
      max_iterations(max_it),
      max_no_improve(max_no_imp)
{
    std::random_device rd;
    rng = std::mt19937(rd());

    // Initialize equal weights for perturbations
    weights = {1.0, 1.0, 1.0, 1.0};
}

// -------------------- Adaptive k --------------------
int AILS::computeK(int current_obj, int best_obj,
                   int no_improve, int iter) {
    int n_items = inst.N;

    int k_min = 1;
    int k_max = std::max(2, n_items / 10);

    double stagnation = (double) no_improve / max_no_improve;
    double distance = 0.0;
    if (best_obj > 0)
        distance = (double)(current_obj - best_obj) / best_obj;
    double progress = (double) iter / max_iterations;

    double intensity = 0.5 * stagnation + 0.3 * (1.0 - progress) + 0.2 * distance;
    intensity = std::min(1.0, std::max(0.0, intensity));

    int k = k_min + (int)((k_max - k_min) * intensity);
    return std::max(1, k);
}

// -------------------- Select perturbation --------------------
int AILS::selectPerturbation() {
    double total = std::accumulate(weights.begin(), weights.end(), 0.0);
    std::uniform_real_distribution<double> dist(0.0, total);
    double r = dist(rng);

    double cumulative = 0.0;
    for (size_t i = 0; i < weights.size(); i++) {
        cumulative += weights[i];
        if (r <= cumulative)
            return i;
    }
    return weights.size() - 1;
}

// -------------------- Apply perturbation --------------------
void AILS::applyPerturbation(int idx, BPPCSolution& sol,
                             int current_obj, int best_obj,
                             int no_improve, int iter) {

    Perturbations pert;
    int k = computeK(current_obj, best_obj, no_improve, iter);

    switch(idx) {
        case 0: pert.relocateK(sol, k); break;
        case 1: pert.exchangeK(sol, k); break;
        case 2: pert.merge(sol); break;
        case 3: pert.split(sol); break;
    }
}

// -------------------- Main AILS --------------------
BPPCSolution AILS::run() {

    SolutionBuilder builder(inst);
    // BPPCSolution current = builder.MFFD();
    // BPPCSolution current = builder.randomFeasible();
    BPPCSolution current = builder.greedy(0.3, K1, K2, K3);

    LocalSearchRVND ls(current, K1, K2, K3);
    ls.run();
    current = ls.getSolution();

    BPPCSolution best = current;
    int best_obj = best.computeObjective(K1, K2, K3);

    int iter = 0;
    int no_improve = 0;

    std::cout << "===== INITIAL AILS RESULT =====\n";
    best.printStatistics(K1, K2, K3);
    std::cout << "\n";

    while (iter < max_iterations && no_improve < max_no_improve) {

        int current_obj = current.computeObjective(K1, K2, K3);

        BPPCSolution candidate = current;
        int p = selectPerturbation();
        applyPerturbation(p, candidate, current_obj, best_obj, no_improve, iter);

        LocalSearchRVND ls_candidate(candidate, K1, K2, K3);
        ls_candidate.run();
        candidate = ls_candidate.getSolution();

        int cand_obj = candidate.computeObjective(K1, K2, K3);

        if (cand_obj < best_obj) {
            best = candidate;
            best_obj = cand_obj;

            current = candidate;
            no_improve = 0;

            // Reward the successful perturbation
            weights[p] += 1.0;
        } else {
            no_improve++;
            // Slight penalty (keep > 0)
            weights[p] = std::max(0.1, weights[p] * 0.95);
        }

        iter++;
    }

    return best;
}
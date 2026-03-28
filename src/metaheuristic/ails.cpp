#include "ails.hpp"
#include "../util/qrvnd.hpp"
#include "../util/rvnd.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <random>

// -------------------- Constructor --------------------
AILS::AILS(const BPPCInstance& instance,
           int k1, int k2, int k3,
           int max_it,
           int max_no_imp,
           bool use_qrvnd_,
           double alpha_, double gamma_, double epsilon_,
           int q_max_it_, int q_max_no_imp_)
    : inst(instance),
      K1(k1), K2(k2), K3(k3),
      max_iterations(max_it),
      max_no_improve(max_no_imp),
      useQRVND(use_qrvnd_),
      alpha(alpha_), gamma(gamma_), epsilon(epsilon_),
      q_max_iterations(q_max_it_), q_max_no_improve(q_max_no_imp_)
{
    std::random_device rd;
    rng = std::mt19937(rd());

    weights = {1.0, 1.0, 1.0, 1.0};
}

// -------------------- Adaptive k --------------------
int AILS::computeK(int current_obj, int best_obj,
                   int no_improve, int iter) {

    int n_items = inst.N;

    int k_min = 1;
    int k_max = std::max(2, n_items / 10);

    double stagnation = (double) no_improve / max_no_improve;
    double distance = (best_obj > 0)
        ? (double)(current_obj - best_obj) / best_obj
        : 0.0;

    double progress = (double) iter / max_iterations;

    double intensity = 0.5 * stagnation
                     + 0.3 * (1.0 - progress)
                     + 0.2 * distance;

    intensity = std::clamp(intensity, 0.0, 1.0);

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
    BPPCSolution current = builder.greedy(0.3, K1, K2, K3);
    BPPCSolution best = current;

    std::cout << "===== INITIAL AILS RESULT =====\n";
    best.printStatistics(K1, K2, K3);
    std::cout << "\n";

    int iter = 0;
    int no_improve = 0;

    // -------------------- INITIAL LOCAL SEARCH --------------------
    if (useQRVND) {
        QRVND ls(current, K1, K2, K3,
                 alpha, gamma, epsilon,
                 q_max_iterations, q_max_no_improve);
        ls.run();
    } else {
        RVND ls(current, K1, K2, K3);
        ls.run();
    }

    best = current;

    // -------------------- MAIN LOOP --------------------
    while (iter < max_iterations && no_improve < max_no_improve) {

        int current_obj = current.computeObjective(K1, K2, K3);
        BPPCSolution candidate = current;

        // ---- Perturbation ----
        int p = selectPerturbation();
        applyPerturbation(p, candidate,
                          current_obj,
                          best.computeObjective(K1,K2,K3),
                          no_improve, iter);

        // ---- Local Search ----
        if (useQRVND) {
            QRVND ls(candidate, K1, K2, K3,
                     alpha, gamma, epsilon,
                     q_max_iterations, q_max_no_improve);
            ls.run();
        } else {
            RVND ls(candidate, K1, K2, K3);
            ls.run();
        }

        int cand_obj = candidate.computeObjective(K1, K2, K3);
        int best_obj = best.computeObjective(K1, K2, K3);

        // ---- Acceptance ----
        if (cand_obj < best_obj) {
            best = candidate;
            current = candidate;
            no_improve = 0;

            weights[p] += 1.0;
        } else {
            no_improve++;
            weights[p] = std::max(0.1, weights[p] * 0.95);
        }

        iter++;
    }

    return best;
}
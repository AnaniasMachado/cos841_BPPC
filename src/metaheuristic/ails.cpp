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
            BuilderType builder_type,
            double beta_,
            bool use_qrvnd_,
            double alpha_, double gamma_, double epsilon_,
            bool verbose_)
    : inst(instance),
      K1(k1), K2(k2), K3(k3),
      max_iterations(max_it),
      max_no_improve(max_no_imp),
      builderType(builder_type),
      beta(beta_),
      useQRVND(use_qrvnd_),
      alpha(alpha_), gamma(gamma_), epsilon(epsilon_),
      verbose(verbose_)
{
    std::random_device rd;
    rng = std::mt19937(rd());

    weights = {1.0, 1.0, 1.0, 1.0};
}

// -------------------- Adaptive k --------------------
int AILS::computeK(int no_improve) {

    int n_items = inst.N;

    // 5% to 20% of items
    int k_min = std::max(1, (int)(0.05 * n_items));
    int k_max = std::max(k_min + 1, (int)(0.20 * n_items));

    // Stagnation in [0,1]
    double stagnation = (double) no_improve / max_no_improve;

    // Fast saturation: >= 0.5 → max intensity
    double intensity = std::min(1.0, stagnation * 2.0);

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
                            int no_improve) {

    Perturbations pert;
    int k = computeK(no_improve);

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
    BPPCSolution current = [&]() {
        switch (builderType) {
            case BuilderType::MFFD:
                return builder.MFFD();

            case BuilderType::RANDOM:
                return builder.randomFeasible();

            case BuilderType::GREEDY:
                return builder.greedy(beta, K1, K2, K3);
        }
        return builder.MFFD();
    }();

    BPPCSolution best = current;

    if (verbose) {
        std::cout << "===== INITIAL AILS RESULT =====\n";
        best.printStatistics(K1, K2, K3);
        std::cout << "\n";
    }

    int iter = 0;
    int no_improve = 0;

    // CREATE ONCE
    QRVND qrvnd(current, K1, K2, K3,
                alpha, gamma, epsilon);

    RVND rvnd(current, K1, K2, K3);

    // -------------------- INITIAL LOCAL SEARCH --------------------
    if (useQRVND) {
        qrvnd.run();
    } else {
        rvnd.run();
    }

    best = current;

    // -------------------- MAIN LOOP --------------------
    while (iter < max_iterations && no_improve < max_no_improve) {

        if (verbose && iter % 5 == 0) {
            int current_obj = current.computeObjective(K1, K2, K3);
            int best_obj = best.computeObjective(K1, K2, K3);

            std::cout << "Iteration: " << iter
                    << " | Current: " << current_obj
                    << " | Best: " << best_obj
                    << " | No improve: " << no_improve
                    << "\n";
        }

        int current_obj = current.computeObjective(K1, K2, K3);
        BPPCSolution candidate = current;

        // ---- Perturbation ----
        int p = selectPerturbation();
        applyPerturbation(p, candidate,
                          no_improve);

        // ---- Local Search ----
        if (useQRVND) {
            qrvnd.setSolution(candidate);
            qrvnd.run();
        } else {
            rvnd.setSolution(candidate);
            rvnd.run();
        }

        int cand_obj = candidate.computeObjective(K1, K2, K3);
        int best_obj = best.computeObjective(K1, K2, K3);

        // ---- Acceptance ----
        if (cand_obj < best_obj) {
            best = candidate;
            current = candidate;
            no_improve = 0;

            if (useQRVND) {
                qrvnd.setSolution(current);
            } else {
                rvnd.setSolution(current);
            }

            weights[p] += 1.0;
        } else {
            no_improve++;
            weights[p] = std::max(0.1, weights[p] * 0.95);
        }

        iter++;
    }

    if (verbose) {
        std::cout << "Iterations to convergence: " << iter << "\n";
    }

    return best;
}
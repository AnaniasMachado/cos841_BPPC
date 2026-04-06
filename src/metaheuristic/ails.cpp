#include "ails.hpp"

// -------------------- Constructor --------------------
AILS::AILS(const BPPCInstance& instance,
            int k1, int k2, int k3,
            int max_it,
            int max_no_imp,
            AcceptanceType acceptance_type_,
            ImprovementType improvement_type_,
            bool use_ucb_, double c_,
            BuilderType builder_type_,
            double beta_,
            bool use_qrvnd_,
            double alpha_, double gamma_, double epsilon_,
            bool verbose_, double time_limit_)
    : inst(instance),
      K1(k1), K2(k2), K3(k3),
      max_iterations(max_it),
      max_no_improve(max_no_imp),
      acceptance_type(acceptance_type_),
      improvement_type(improvement_type_),
      useUCB(use_ucb_), c(c_),
      builder_type(builder_type_),
      beta(beta_),
      useQRVND(use_qrvnd_),
      alpha(alpha_), gamma(gamma_), epsilon(epsilon_),
      verbose(verbose_), time_limit(time_limit_)
{
    std::random_device rd;
    rng = std::mt19937(rd());

    perturbation_count = std::vector<int>(4, 0);
    perturbation_success = std::vector<int>(4, 0);
}

// -------------------- Adaptive k --------------------
int AILS::computeK(PerturbationType perturbation_type, BPPCSolution& sol, int no_improve) {

    int n_items = inst.N;
    int n_bins = sol.bins.size();

    // Stagnation in [0,1]
    double stagnation = (double) no_improve / max_no_improve;

    if ((PerturbationType::RELOCATEK == perturbation_type) ||
        (PerturbationType::EXCHANGEK == perturbation_type)) {
        // 5% to 20% of items
        int k_min = std::max(1, (int)(0.05 * n_items));
        int k_max = std::max(k_min + 1, (int)(0.20 * n_items));

        // Fast saturation: >= 0.5 implies max intensity
        double intensity = std::min(1.0, stagnation * 2.0);

        int k = k_min + (int)((k_max - k_min) * intensity);

        return std::max(1, k);
    } else if (PerturbationType::MERGEK == perturbation_type) {
        // 2.5% to 5% of bins
        int k_min = std::max(2, (int)(0.025 * n_bins));
        int k_max = std::max(k_min + 1, (int)(0.05 * n_bins));

        // Fast saturation: >= 0.5 implies max intensity
        double intensity = std::min(1.0, stagnation * 2.0);

        int k = k_min + (int)((k_max - k_min) * intensity);

        return std::max(1, k);
    } else if (PerturbationType::SPLITK == perturbation_type) {
        // 1% to 2% of bins
        int k_min = std::max(1, (int)(0.01 * n_bins));
        int k_max = std::max(k_min + 1, (int)(0.02 * n_bins));

        // Fast saturation: >= 0.5 implies max intensity
        double intensity = std::min(1.0, stagnation * 2.0);

        int k = k_min + (int)((k_max - k_min) * intensity);

        return std::max(1, k);
    }
}

// -------------------- Switch perturbation --------------------
PerturbationType AILS::getPerturbationType(int idx) {
    switch(idx) {
        case 0: return PerturbationType::RELOCATEK;
        case 1: return PerturbationType::EXCHANGEK;
        case 2: return PerturbationType::MERGEK;
        case 3: return PerturbationType::SPLITK;
    }
}

// -------------------- Select perturbation --------------------
PerturbationType AILS::selectPerturbation(int iter) {
    // Number of perturbations
    int n = 4;

    // If some operator was never tried then try it first
    for (int i = 0; i < n; i++) {
        if (perturbation_count[i] == 0) {
            return getPerturbationType(i);
        }
    }

    double best_score = std::numeric_limits<double>::lowest();
    int best = 0;

    for (int i = 0; i < n; i++) {
        double mean = (double) perturbation_success[i] / perturbation_count[i];

        double bonus = c * sqrt(log(iter) / perturbation_count[i]);

        double score = mean + bonus;

        if (score > best_score) {
            best_score = score;
            best = i;
        }
    }

    return getPerturbationType(best);
}

// -------------------- Apply perturbation --------------------
void AILS::applyPerturbation(PerturbationType perturbation_type, BPPCSolution& sol,
                            int no_improve) {

    Perturbations pert;
    int k = computeK(perturbation_type, sol, no_improve);

    switch(perturbation_type) {
        case PerturbationType::RELOCATEK: pert.relocateK(sol, k); break;
        case PerturbationType::EXCHANGEK: pert.exchangeK(sol, k); break;
        case PerturbationType::MERGEK: pert.mergeK(sol, k); break;
        case PerturbationType::SPLITK: pert.splitK(sol, k); break;
    }
}

// -------------------- Update weights --------------------
void AILS::updateUCB(PerturbationType perturbation_type, bool reward) {
    int p = 0;
    switch(perturbation_type) {
        case PerturbationType::RELOCATEK: p = 0; break;
        case PerturbationType::EXCHANGEK: p = 1; break;
        case PerturbationType::MERGEK: p = 2; break;
        case PerturbationType::SPLITK: p = 3; break;
    }

    ++perturbation_count[p];
    if (reward) ++perturbation_success[p];
}

// -------------------- Main AILS --------------------
BPPCSolution AILS::run() {

    auto start = std::chrono::high_resolution_clock::now();

    SolutionBuilder builder(inst);
    BPPCSolution current = [&]() {
        switch (builder_type) {
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

    RVND rvnd(current, improvement_type, K1, K2, K3);

    QRVND qrvnd(current, improvement_type, K1, K2, K3,
                alpha, gamma, epsilon);

    // -------------------- INITIAL LOCAL SEARCH --------------------
    if (useQRVND) {
        qrvnd.run();
    } else {
        rvnd.run();
    }

    best = current;

    // -------------------- MAIN LOOP --------------------
    while (iter < max_iterations && no_improve < max_no_improve) {

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();

        if (elapsed > time_limit) {
            if (verbose) std::cout << "ExceedTimeLimit: " << elapsed << "\n";
            break;
        }

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
        PerturbationType perturbation_type = selectPerturbation(iter);
        applyPerturbation(perturbation_type, candidate, no_improve);

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
            no_improve = 0;
            if (useUCB) updateUCB(perturbation_type, true);
        } else {
            no_improve++;
            if (useUCB) updateUCB(perturbation_type, false);
        }

        // ---- Update CURRENT based on acceptance policy ----
        if ((acceptance_type == AcceptanceType::BEST && cand_obj < best_obj) ||
            (acceptance_type == AcceptanceType::ITERATIVE && cand_obj < current_obj) ||
            (acceptance_type == AcceptanceType::RW)) {

            current = candidate;

            if (useQRVND) {
                qrvnd.setSolution(current);
            } else {
                rvnd.setSolution(current);
            }
        }

        iter++;
    }

    if (verbose) {
        std::cout << "Iterations to convergence: " << iter << "\n";
        for (int i = 0; i < perturbation_count.size(); ++i) {
            std::cout << "Perturbation: " << i 
                    << "; Count: " << perturbation_count[i] 
                    << "; Success: " << perturbation_success[i] 
                    << "\n";
        }
    }

    return best;
}
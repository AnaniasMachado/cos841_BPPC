#include "ils.hpp"

#include <algorithm>
#include <numeric>
#include <unordered_set>

ILS::ILS(const BPPCSolution& initial_solution,
         ImprovementType improvement_type,
         int k1, int k2, int k3,
         const Params& params,
         bool verbose_,
         double time_limit_,
         int max_outer_iterations_)
    : params_(params),
      K1_(k1),
      K2_(k2),
      K3_(k3),
      verbose(verbose_),
      time_limit(time_limit_),
      max_outer_iterations(max_outer_iterations_),
      rng_(params.seed),
      s_best_(initial_solution),
      ls_(s_best_, improvement_type, k1, k2, k3) {}

int ILS::computeLowerBound(const BPPCSolution& s) const {
    int total_weight = 0;
    for (int w : s.weights) {
        total_weight += w;
    }
    return (total_weight + s.C - 1) / s.C;
}

int ILS::cost(const BPPCSolution& s) const {
    return s.computeObjective(K1_, K2_, K3_);
}

std::vector<int> ILS::collectAllItems(const BPPCSolution& s) const {
    std::vector<int> items(s.N);
    std::iota(items.begin(), items.end(), 0);
    return items;
}

std::vector<int> ILS::sampleWithoutReplacement(const std::vector<int>& source, int k) {
    if (k <= 0 || source.empty()) {
        return {};
    }

    std::vector<int> tmp = source;
    std::shuffle(tmp.begin(), tmp.end(), rng_);

    if (k > static_cast<int>(tmp.size())) {
        k = static_cast<int>(tmp.size());
    }

    tmp.resize(k);
    return tmp;
}

BPPCSolution ILS::reduceNbBins(const BPPCSolution& s) {
    BPPCSolution reduced = s;

    if (reduced.bins.size() <= 1) {
        return reduced;
    }

    std::uniform_int_distribution<int> bin_dist(0, static_cast<int>(reduced.bins.size()) - 1);
    const int removed_bin = bin_dist(rng_);

    std::vector<std::vector<int>> new_bins = reduced.bins;
    std::vector<int> removed_items = new_bins[removed_bin];
    new_bins.erase(new_bins.begin() + removed_bin);

    if (new_bins.empty()) {
        return reduced;
    }

    std::shuffle(removed_items.begin(), removed_items.end(), rng_);
    std::uniform_int_distribution<int> target_dist(0, static_cast<int>(new_bins.size()) - 1);

    for (int item : removed_items) {
        const int target_bin = target_dist(rng_);
        new_bins[target_bin].push_back(item);
    }

    reduced.rebuildSolutionFromBins(new_bins);
    return reduced;
}

void ILS::localSearchPhase(BPPCSolution& s) {
    bool verbose = false;

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    ls_.setSolution(s);

    double total_classic = 0.0;
    double total_assignment = 0.0;
    double total_ejection = 0.0;
    double total_grenade = 0.0;

    auto phase_start = std::chrono::high_resolution_clock::now();

    for (int it = 0; it < params_.n_ls; ++it) {
        auto t0 = std::chrono::high_resolution_clock::now();
        ls_.classic();
        auto t1 = std::chrono::high_resolution_clock::now();
        total_classic += std::chrono::duration<double>(t1 - t0).count();

        t0 = std::chrono::high_resolution_clock::now();
        ls_.assignment(params_.n_assign);
        t1 = std::chrono::high_resolution_clock::now();
        total_assignment += std::chrono::duration<double>(t1 - t0).count();

        t0 = std::chrono::high_resolution_clock::now();
        ls_.ejectionChain();
        t1 = std::chrono::high_resolution_clock::now();
        total_ejection += std::chrono::duration<double>(t1 - t0).count();

        t0 = std::chrono::high_resolution_clock::now();
        ls_.grenade();
        t1 = std::chrono::high_resolution_clock::now();
        total_grenade += std::chrono::duration<double>(t1 - t0).count();

        // ---- Progress log every 5 iterations ----
        if (it % 5 == 0) {
            log("[ILS] LS progress | it=" + std::to_string(it) +
                "/" + std::to_string(params_.n_ls) +
                " | obj=" + std::to_string(s.computeObjective(K1_, K2_, K3_)) +
                " | bins=" + std::to_string(s.binsUsed()) +
                " | feasible=" + std::string(s.isFeasible() ? "yes" : "no"));
        }
    }

    auto phase_end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(phase_end - phase_start).count();

    // ---- Summary log ----
    log("[ILS] LS summary | total_time=" + std::to_string(total_time) + " s" +
        " | classic=" + std::to_string(total_classic) +
        " | assignment=" + std::to_string(total_assignment) +
        " | ejection=" + std::to_string(total_ejection) +
        " | grenade=" + std::to_string(total_grenade));
}

void ILS::maybeSetCovering(BPPCSolution& s) {
    ls_.setSolution(s);
    ls_.updateK();
    ls_.setCoveringBinFeasible();
}

void ILS::updatePoolFromLocalMinimum(BPPCSolution& s) {
    ls_.setSolution(s);
    ls_.updateK();
    ls_.updatePool();
}

BPPCSolution ILS::shaking(const BPPCSolution& s) {
    BPPCSolution shaken = s;

    if (shaken.bins.size() <= 1 || params_.shake_size <= 0) {
        return shaken;
    }

    auto rand_int = [&](int l, int r) {
        std::uniform_int_distribution<int> dist(l, r);
        return dist(rng_);
    };

    // -------------------- Collect candidate problematic items from bad bins --------------------
    std::vector<int> problematic_pool;
    std::vector<char> used_item(shaken.N, 0);

    for (int b : shaken.bad_bins) {
        if (b < 0 || b >= (int)shaken.bins.size()) continue;

        for (int item : shaken.bins[b]) {
            if (!used_item[item]) {
                used_item[item] = 1;
                problematic_pool.push_back(item);
            }
        }
    }

    // -------------------- Select ~50% problematic items --------------------
    int want_problematic = params_.shake_size / 2;
    std::shuffle(problematic_pool.begin(), problematic_pool.end(), rng_);

    std::vector<int> selected;
    for (int i = 0; i < std::min(want_problematic, (int)problematic_pool.size()); ++i) {
        selected.push_back(problematic_pool[i]);
    }

    // Mark already selected
    std::vector<char> chosen(shaken.N, 0);
    for (int item : selected) {
        chosen[item] = 1;
    }

    // -------------------- Fill remaining slots with random items --------------------
    std::vector<int> random_pool;
    random_pool.reserve(shaken.N);

    for (int item = 0; item < shaken.N; ++item) {
        if (!chosen[item]) {
            random_pool.push_back(item);
        }
    }

    std::shuffle(random_pool.begin(), random_pool.end(), rng_);

    int remaining = params_.shake_size - (int)selected.size();
    for (int i = 0; i < std::min(remaining, (int)random_pool.size()); ++i) {
        selected.push_back(random_pool[i]);
    }

    // -------------------- Randomize relocation order --------------------
    std::shuffle(selected.begin(), selected.end(), rng_);

    // -------------------- Relocate selected items --------------------
    for (int item : selected) {
        int from_bin = shaken.item_bin[item];

        if (from_bin < 0 || from_bin >= (int)shaken.bins.size()) {
            continue;
        }

        if ((int)shaken.bins.size() <= 1) {
            break;
        }

        int to_bin = rand_int(0, (int)shaken.bins.size() - 2);
        if (to_bin >= from_bin) {
            ++to_bin;
        }

        shaken.moveItem(item, from_bin, to_bin);
    }

    return shaken;
}

BPPCSolution ILS::run() {
    auto start = std::chrono::high_resolution_clock::now();

    auto log = [&](const std::string& msg) {
        if (verbose) std::cout << msg << "\n";
    };

    BPPCSolution best_feasible = s_best_;
    const int k_lb = computeLowerBound(best_feasible);

    int outer_it = 0;

    if (verbose) {
        log("----- ILS START -----");
        log("Initial | Obj: " + std::to_string(cost(best_feasible)) +
            " | Bins: " + std::to_string(best_feasible.binsUsed()) +
            " | LB: " + std::to_string(k_lb));
    }

    while (best_feasible.binsUsed() > k_lb && best_feasible.isFeasible()) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - start).count();

        if (elapsed > time_limit) {
            if (verbose) {
                log("----- ILS STOP -----");
                log("Reason: time limit");
                log("Elapsed: " + std::to_string(elapsed));
            }
            break;
        }

        if (outer_it >= max_outer_iterations) {
            if (verbose) {
                log("----- ILS STOP -----");
                log("Reason: max outer iterations");
                log("Outer iterations: " + std::to_string(outer_it));
            }
            break;
        }

        outer_it++;

        if (verbose) {
            log("----- OUTER ITERATION " + std::to_string(outer_it) + " -----");
            log("Current feasible | Obj: " + std::to_string(cost(best_feasible)) +
                " | Bins: " + std::to_string(best_feasible.binsUsed()));
        }

        BPPCSolution s_best = reduceNbBins(best_feasible);
        BPPCSolution s = s_best;
        int i_shak = 0;

        if (verbose) {
            log("Reduced | Obj: " + std::to_string(cost(s_best)) +
                " | Bins: " + std::to_string(s_best.binsUsed()) +
                " | Feasible: " + std::string(s_best.isFeasible() ? "yes" : "no"));
        }

        while (i_shak < params_.max_shakes && !s_best.isFeasible()) {
            now = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration<double>(now - start).count();

            if (elapsed > time_limit) {
                if (verbose) {
                    log("----- ILS STOP -----");
                    log("Reason: time limit");
                    log("Elapsed: " + std::to_string(elapsed));
                }
                return best_feasible;
            }

            if (verbose && i_shak % 5 == 0) {
                log("Shake: " + std::to_string(i_shak) +
                    " | Current: " + std::to_string(cost(s)) +
                    " | BestReduced: " + std::to_string(cost(s_best)));
            }

            localSearchPhase(s);

            if (i_shak > 0 && i_shak % params_.n_sc == 0) {
                if (verbose) {
                    log("SetCovering | Shake: " + std::to_string(i_shak));
                }
                maybeSetCovering(s);
            }

            if (cost(s) < cost(s_best)) {
                s_best = s;
                i_shak = 0;

                if (verbose) {
                    log("Improved reduced | Obj: " + std::to_string(cost(s_best)) +
                        " | Bins: " + std::to_string(s_best.binsUsed()) +
                        " | Feasible: " + std::string(s_best.isFeasible() ? "yes" : "no"));
                }
            } else {
                i_shak++;
            }

            updatePoolFromLocalMinimum(s_best);
            s = shaking(s_best);
        }

        if (verbose) {
            log("End reduced phase | Obj: " + std::to_string(cost(s_best)) +
                " | Bins: " + std::to_string(s_best.binsUsed()) +
                " | Feasible: " + std::string(s_best.isFeasible() ? "yes" : "no"));
        }

        if (s_best.isFeasible() && cost(s_best) < cost(best_feasible)) {
            best_feasible = s_best;
            s_best_ = best_feasible;

            if (verbose) {
                log("Accepted feasible best | Obj: " + std::to_string(cost(best_feasible)) +
                    " | Bins: " + std::to_string(best_feasible.binsUsed()));
            }
        } else {
            if (verbose) {
                log("----- ILS STOP -----");
                log("Reason: no better feasible solution after reduction");
            }
            break;
        }
    }

    if (verbose) {
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();

        log("----- ILS END -----");
        log("Final | Obj: " + std::to_string(cost(best_feasible)) +
            " | Bins: " + std::to_string(best_feasible.binsUsed()) +
            " | Feasible: " + std::string(best_feasible.isFeasible() ? "yes" : "no"));
        log("Outer iterations: " + std::to_string(outer_it));
        log("Elapsed: " + std::to_string(elapsed));
    }

    return best_feasible;
}
#ifndef LOCAL_SEARCH_HPP
#define LOCAL_SEARCH_HPP

#include "solution.hpp"
#include <algorithm>
#include <random>
#include <climits>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <deque>
#include "highs/Highs.h"
#include "highs/lp_data/HighsLp.h"
#include "gurobi_c++.h"

enum class ImprovementType {
    FI,
    BI
};

class LocalSearch {
private:
    struct Column {
        std::vector<int> items;
        std::vector<uint8_t> mask;  // fast membership lookup
        int cost = 0;

        inline bool contains(int item) const {
            return mask[item];
        }

        void buildMask(int n_items) {
            mask.assign(n_items, 0);
            for (int x : items) {
                mask[x] = 1;
            }
        }
    };

    struct PoolEntry {
        Column col;
        size_t hash;
    };

    int K;

    std::deque<PoolEntry> pool;
    std::unordered_set<size_t> seen;
    int S_pool;
    int S_min;
    int S_max;

    std::vector<std::vector<int>> elite_bins;
    std::unordered_set<size_t> elite_hashes;
    size_t elite_solution_hash = 0; 

    std::unordered_set<size_t> tabu_solutions;
    size_t last_solution_hash = 0;
    int tabu_tenure = 20;
    std::deque<size_t> tabu_queue;

    BPPCSolution* sol;
    int K1, K2, K3;
    ImprovementType improvement_type;
    std::mt19937 rng;

    int computeObjective(const BPPCSolution& s) const;
    size_t hashBin(const std::vector<int>& bin) const;
    size_t hashSolution(const std::vector<std::vector<int>>& bins) const;
    void addColumnAllowInfeasible(Column&& col, int n_items);
    void addColumn(Column&& col, int n_items);
    void trimPool();
    bool isTabu(size_t h);
    void addTabu(size_t h);
    void updatePoolSize(bool optimal_solved);
    std::vector<std::vector<int>> repairSolution(
        const std::vector<std::vector<int>>& input_bins);

public:
    LocalSearch(BPPCSolution& solution, ImprovementType improvement, int k1, int k2, int k3);

    bool relocation();
    bool exchange();
    bool exchange21();
    bool exchange22();
    bool exchange32();
    bool classic();
    bool bestMoveForPair(int b1, int b2, bool allow_zero_cost);
    bool classic_ILS();
    bool add();
    bool ejection();
    bool ejectionGreedy();
    bool ejectionGC();
    bool ejectionGlobal();
    int hungarian(const std::vector<std::vector<int>>& cost,
                    std::vector<int>& assignment);
    bool assignment(int N_ASSIGN);
    bool repackingGreedy(int N_ATTEMPTS);
    bool dualPhaseMove(int N_ASSIGN, int N_ATTEMPTS);
    bool setCoveringLPFeasible(int K_upper);
    bool setCoveringVanilla();
    bool setCoveringBinFeasible();
    bool kempeChain();
    bool ejectionChain();
    bool grenade();

    void updateK();
    void updateElite();

    void updatePool();
    void addToPool(const BPPCSolution& s);
    void generateColumns();

    void setSolution(BPPCSolution& solution);
};

#endif
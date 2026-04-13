#ifndef LOCAL_SEARCH_HPP
#define LOCAL_SEARCH_HPP

#include "solution.hpp"
#include <algorithm>
#include <random>
#include <climits>
#include <unordered_map>
#include "highs/Highs.h"
#include "highs/lp_data/HighsLp.h"

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

    std::deque<PoolEntry> pool;
    std::unordered_set<size_t> seen;
    int S_pool;
    int S_min;
    int S_max;

    std::unordered_set<size_t> tabu_solutions;
    size_t last_solution_hash = 0;
    int tabu_tenure = 20;
    std::deque<size_t> tabu_queue;

    BPPCSolution* sol;
    int K1, K2, K3;
    ImprovementType improvement_type;
    std::mt19937 rng;

    int computeObjective(const BPPCSolution& s) const;
    size_t hash_solution(const std::vector<std::vector<int>>& bins) const;
    size_t hash_pool() const;
    bool isTabu(size_t h);
    void addTabu(size_t h);
    void updatePoolSize(bool optimal_solved);

public:
    LocalSearch(BPPCSolution& solution, ImprovementType improvement, int k1, int k2, int k3);

    bool relocation();
    bool exchange();
    bool add();
    bool ejection();
    bool ejectionGreedy();
    bool ejectionGC();
    bool ejectionGlobal();
    int hungarian(const std::vector<std::vector<int>>& cost,
                    std::vector<int>& assignment);
    bool assignment();
    bool setCovering();
    void updatePool();

    void setSolution(BPPCSolution& solution);
};

#endif
#include <iostream>
#include "../util/solution.hpp"

int main() {
    // Example problem with 5 items
    int N = 5;
    int C = 10;

    std::vector<int> weights = {3, 5, 4, 6, 2};

    // -------------------- Build conflicts --------------------
    std::vector<std::unordered_set<int>> conflicts_set(N);

    conflicts_set[0].insert(2); // item 0 conflicts with 2
    conflicts_set[1].insert(3); // item 1 conflicts with 3

    // -------------------- Convert to Bitset format --------------------
    using Bitset = std::vector<uint64_t>;

    int W = (N + 63) / 64;
    std::vector<Bitset> conflicts_bitset(N, Bitset(W, 0ULL));

    for (int i = 0; i < N; i++) {
        for (int j : conflicts_set[i]) {
            conflicts_bitset[i][j >> 6] |= (1ULL << (j & 63));
            conflicts_bitset[j][i >> 6] |= (1ULL << (i & 63));
        }
    }

    // -------------------- Create solution --------------------
    BPPCSolution sol(N, C, weights, conflicts_bitset);

    sol.addItemToBin(0, 0);
    sol.addItemToBin(1, 0);
    sol.addItemToBin(2, 1);
    sol.addItemToBin(3, 1);
    sol.addItemToBin(4, 1);

    sol.print();

    double obj = sol.computeObjective(1.0, 2.0, 5.0);
    std::cout << "Objective value: " << obj << "\n";

    return 0;
}
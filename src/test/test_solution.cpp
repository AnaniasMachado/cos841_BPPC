#include <iostream>
#include "../util/solution.hpp"

int main() {
    // Example problem with 5 items
    int N = 5;
    int C = 10;
    std::vector<int> weights = {0, 3, 5, 4, 6, 2}; // 1-based indexing
    std::vector<std::unordered_set<int>> conflicts(N+1);
    conflicts[1].insert(3); // item 1 conflicts with 3
    conflicts[2].insert(4);

    BPPCSolution sol(N, C, weights, conflicts);

    sol.addItemToBin(1, 0);
    sol.addItemToBin(2, 0);
    sol.addItemToBin(3, 1);
    sol.addItemToBin(4, 1);
    sol.addItemToBin(5, 1);

    sol.print();

    double obj = sol.computeObjective(1.0, 2.0, 5.0);
    std::cout << "Objective value: " << obj << "\n";

    return 0;
}
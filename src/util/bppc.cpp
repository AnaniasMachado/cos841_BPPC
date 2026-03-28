#include "bppc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void BPPCInstance::print() const {
    cout << "N = " << N << ", C = " << C << "\n";
    for (int i = 1; i <= N; i++) {
        cout << "Item " << i << ": w=" << weights[i] << ", conflicts={ ";
        for (int j : conflicts[i]) {
            cout << j << " ";
        }
        cout << "}\n";
    }
}

void BPPCInstance::printStatistics() const {
    // Number of edges in the conflict graph
    int edge_count = 0;
    for (int i = 1; i <= N; i++) {
        edge_count += conflicts[i].size();
    }
    edge_count /= 2; // Each edge counted twice

    std::cout << "Instance statistics:\n";
    std::cout << "Number of items (N): " << N << "\n";
    std::cout << "Bin capacity (C): " << C << "\n";
    std::cout << "Number of conflicts (|E|): " << edge_count << "\n";
}

BPPCInstance readInstance(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file.\n";
        exit(1);
    }

    int N, C;
    file >> N >> C;

    vector<int> weights(N + 1, 0);
    vector<unordered_set<int>> conflicts(N + 1);

    string line;
    getline(file, line); // consume rest of first line

    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        int i, w;
        ss >> i >> w;

        weights[i] = w;

        int a;
        while (ss >> a) {
            conflicts[i].insert(a);
            conflicts[a].insert(i);
        }
    }

    return {N, C, weights, conflicts};
}
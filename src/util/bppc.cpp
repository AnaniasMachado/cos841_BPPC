#include "bppc.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void BPPCInstance::print() const {
    cout << "N = " << N << ", C = " << C << "\n";

    for (int i = 0; i < N; i++) {
        cout << "Item " << i << ": w=" << weights[i] << ", conflicts={ ";

        for (int j = 0; j < N; j++) {
            int word = j >> 6;
            int bit = j & 63;

            if (conflicts[i][word] & (1ULL << bit)) {
                cout << j << " ";
            }
        }

        cout << "}\n";
    }
}

void BPPCInstance::printStatistics() const {
    long long edge_count = 0;

    for (int i = 0; i < N; i++) {
        for (uint64_t w : conflicts[i]) {
            edge_count += __builtin_popcountll(w);
        }
    }

    edge_count /= 2;

    cout << "Instance statistics:\n";
    cout << "Number of items (N): " << N << "\n";
    cout << "Bin capacity (C): " << C << "\n";
    cout << "Number of conflicts (|E|): " << edge_count << "\n";
}

BPPCInstance readInstance(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file.\n";
        exit(1);
    }

    int N, C;
    file >> N >> C;

    vector<int> weights(N, 0);

    int W = (N + 63) / 64;
    vector<vector<uint64_t>> conflicts(N, vector<uint64_t>(W, 0ULL));

    int i, w;
    while (file >> i >> w) {
        i--;

        weights[i] = w;

        int a;
        string line;
        getline(file, line);
        stringstream ss(line);

        while (ss >> a) {
            a--;

            if (a < 0 || a >= N) continue;

            conflicts[i][a >> 6] |= (1ULL << (a & 63));
            conflicts[a][i >> 6] |= (1ULL << (i & 63));
        }
    }

    return {N, C, weights, conflicts};
}
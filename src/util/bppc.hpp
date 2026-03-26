#ifndef BPPC_H
#define BPPC_H

#include <vector>
#include <unordered_set>
#include <string>

struct BPPCInstance {
    int N;
    int C;
    std::vector<int> weights;
    std::vector<std::unordered_set<int>> conflicts;

    void print() const;
};

// Function declaration
BPPCInstance readInstance(const std::string& filename);

#endif
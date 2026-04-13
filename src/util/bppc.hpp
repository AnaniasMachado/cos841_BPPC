#ifndef BPPC_H
#define BPPC_H

#include <vector>
#include <string>
#include <cstdint>

struct BPPCInstance {
    int N;
    int C;
    std::vector<int> weights;
    std::vector<std::vector<uint64_t>> conflicts;

    void print() const;

    void printStatistics() const;
};

BPPCInstance readInstance(const std::string& filename);

#endif
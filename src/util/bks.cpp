#include "bks.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// -------------------- Constructor --------------------
BKSLoader::BKSLoader(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening BKS file\n";
        return;
    }

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        std::string instance;
        int bks, opt;

        ss >> instance >> bks >> opt;

        table[instance] = {bks, opt};
    }
}

// -------------------- Methods --------------------
bool BKSLoader::hasInstance(const std::string& instance) const {
    return table.find(instance) != table.end();
}

BKSData BKSLoader::get(const std::string& instance) const {
    return table.at(instance);
}

BKSResult BKSLoader::evaluate(const std::string& instance, int solution_bins) const {
    BKSResult res{};

    auto it = table.find(instance);
    if (it == table.end()) {
        res.bks = -1;
        res.gap = -1;
        res.reach_bks = 0;
        res.reach_opt = 0;
        return res;
    }

    int bks = it->second.bks;
    int opt = it->second.opt;

    res.bks = bks;
    res.gap = 100.0 * (solution_bins - bks) / bks;
    res.reach_bks = (solution_bins == bks) ? 1 : 0;
    res.reach_opt = (solution_bins == bks && opt == 1) ? 1 : 0;

    return res;
}

// -------------------- Utility: Extract instance name --------------------
std::string extractInstanceName(const std::string& path) {
    size_t pos = path.find("BPPC/");
    if (pos == std::string::npos) return "";

    std::string sub = path.substr(pos + 5); // remove "BPPC/"
    
    // remove ".txt"
    if (sub.size() > 4 && sub.substr(sub.size() - 4) == ".txt") {
        sub = sub.substr(0, sub.size() - 4);
    }

    return sub;
}
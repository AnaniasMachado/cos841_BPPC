#ifndef BKS_HPP
#define BKS_HPP

#include <string>
#include <unordered_map>

struct BKSData {
    int bks;
    int opt;
};

struct BKSResult {
    int bks;
    double gap;
    int reach_bks;
    int reach_opt;
};

class BKSLoader {
public:
    BKSLoader(const std::string& filename);

    bool hasInstance(const std::string& instance) const;

    BKSData get(const std::string& instance) const;

    BKSResult evaluate(const std::string& instance, int solution_bins) const;

private:
    std::unordered_map<std::string, BKSData> table;
};

// -------------------- Utility function --------------------
std::string extractInstanceName(const std::string& path);

#endif
#ifndef EXPERIMENT_RUNNER_HPP
#define EXPERIMENT_RUNNER_HPP

#include <string>
#include <vector>
#include <filesystem>

#include "bppc.hpp"
#include "solution.hpp"

struct RunResult {
    int obj;
    double time;
    double gap;
    bool feasible;
};

class ExperimentRunner {
public:
    // Constructor
    explicit ExperimentRunner(const std::string& root_dir);

    // Instance handling
    std::vector<std::string> getAllInstancePaths() const;

    std::string extractRelativePath(const std::string& fullPath) const;

    std::string buildOutputFolder(const std::string& relativePath) const;

    void ensureDirectory(const std::string& path) const;

    int countExistingRuns(const std::string& folder,
                          const std::string& instance_name) const;

    void saveSolutionToFile(const std::string& filepath,
                            const BPPCSolution& sol,
                            double time,
                            int obj,
                            bool feasible) const;

private:
    std::string root;

    bool isValidInstancePath(const std::string& path) const;
};

#endif
#include "experiment_runner.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

// -------------------- Constructor --------------------
ExperimentRunner::ExperimentRunner(const std::string& root_dir)
    : root(root_dir) {}

// -------------------- Path filter --------------------
bool ExperimentRunner::isValidInstancePath(const std::string& full_path) const {
    std::string path = full_path;
    std::replace(path.begin(), path.end(), '\\', '/');

    // Skip ELGN instances
    if (path.find("/t/ELGN/") != std::string::npos) return false;
    if (path.find("/u/ELGN/") != std::string::npos) return false;

    return true;
}

// -------------------- Collect instances --------------------
std::vector<std::string> ExperimentRunner::getAllInstancePaths() const {
    std::vector<std::string> paths;

    for (const auto& entry : fs::recursive_directory_iterator(root)) {

        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".txt") continue;

        std::string full_path = entry.path().string();

        if (!isValidInstancePath(full_path)) continue;

        paths.push_back(full_path);
    }

    return paths;
}

// -------------------- Relative path --------------------
std::string ExperimentRunner::extractRelativePath(const std::string& fullPath) const {
    fs::path full(fullPath);
    fs::path base(root);

    return fs::relative(full.parent_path(), base).string();
}

// -------------------- Output folder --------------------
std::string ExperimentRunner::buildOutputFolder(const std::string& relativePath) const {
    return "../solutions/ails/" + relativePath;
}

// -------------------- Ensure directory --------------------
void ExperimentRunner::ensureDirectory(const std::string& path) const {
    fs::create_directories(path);
}

// -------------------- Count existing runs --------------------
int ExperimentRunner::countExistingRuns(const std::string& folder,
                                        const std::string& instance_name) const {

    if (!fs::exists(folder)) return 0;

    int count = 0;

    for (const auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;

        std::string name = entry.path().filename().string();

        if (name.find(instance_name) != std::string::npos &&
            name.find("sol_") == 0) {
            count++;
        }
    }

    return count;
}

// -------------------- Save solution --------------------
void ExperimentRunner::saveSolutionToFile(const std::string& filepath,
                                          const BPPCSolution& sol,
                                          double time,
                                          int obj,
                                          bool feasible) const {

    std::ofstream out(filepath);

    if (!out) {
        std::cerr << "Error opening file: " << filepath << "\n";
        return;
    }

    out << "# Objective: " << obj << "\n";
    out << "# Time: " << time << "\n";
    out << "# Feasible: " << feasible << "\n";
    out << "# Solution:\n";

    const auto& bins = sol.bins;

    for (size_t i = 0; i < bins.size(); i++) {
        out << "Bin " << i << ": ";
        for (auto item : bins[i]) {
            out << item << " ";
        }
        out << "\n";
    }

    out.close();
}
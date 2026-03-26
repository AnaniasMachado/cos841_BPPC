#include <iostream>
#include <string>
#include "../util/bppc.hpp"

int main() {
    std::string instance_folder = "../instances/BPPC_test_instances/BPPC";
    std::string instance_type = "/d";
    std::string instance_name = "/BPWC_1_1_1.txt";

    std::string filename = instance_folder + instance_type + instance_name;

    BPPCInstance instance = readInstance(filename);

    std::cout << "Instance successfully read!\n\n";
    instance.print();

    std::cout << "\n--- Quick checks ---\n";
    std::cout << "Number of items: " << instance.N << "\n";
    std::cout << "Capacity: " << instance.C << "\n";
    std::cout << "Weight of item 1: " << instance.weights[1] << "\n";

    std::cout << "Conflicts of item 1: { ";
    for (int x : instance.conflicts[1]) {
        std::cout << x << " ";
    }
    std::cout << "}\n";

    return 0;
}
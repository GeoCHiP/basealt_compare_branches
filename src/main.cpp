#include <iostream>
#include <fstream>

#include "branch_operations.hpp"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage:\n";
        std::cout << argv[0] << " branch1 branch2\n";
        std::cout << "branch1\t\t- Name of the first branch.\n";
        std::cout << "branch2\t\t- name of the second branch.\n";
        exit(EXIT_FAILURE);
    }

    std::string branch1(argv[1]);
    std::string branch2(argv[2]);

    nlohmann::json data = BranchBinaryPackages(branch1);
    auto b1ArchToNamesToInfo = Json2Map(data);

    data = BranchBinaryPackages(branch2);
    auto b2ArchToNamesToInfo = Json2Map(data);

    data.clear();

    nlohmann::json result;

    FirstNotSecond(b1ArchToNamesToInfo, b2ArchToNamesToInfo, "first_not_second", result, true);
    FirstNotSecond(b2ArchToNamesToInfo, b1ArchToNamesToInfo, "second_not_first", result);

    std::ofstream outFile("comparison_" + branch1 + '_' + branch2 + ".json");
    outFile << std::setw(4) << result;

    return 0;
}


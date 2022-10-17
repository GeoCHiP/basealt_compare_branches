#pragma once

#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>


using ArchToNamesToInfo = std::unordered_map<std::string, std::unordered_map<std::string, nlohmann::json>>;

nlohmann::json BranchBinaryPackages(const std::string &branch, const std::string &arch = "");

ArchToNamesToInfo Json2Map(const nlohmann::json &data);

void FirstNotSecond(const ArchToNamesToInfo &b1, const ArchToNamesToInfo &b2,
                    const std::string &pLabel, nlohmann::json &o_Result,
                    bool checkVersionRelease = false,
                    const std::string &vrLabel = "version-release_greater_first");


#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>

#include <curl/curl.h>

#include "vendor/json/json.hpp"

//  libcurl variables for error strings and returned data
static char errorBuffer[CURL_ERROR_SIZE];
static std::string buffer;
 
//  libcurl write callback function
static int writer(char *data, size_t size, size_t nmemb,
                  std::string *writerData) {
    if (writerData == NULL)
        return 0;

    writerData->append(data, size * nmemb);

    return size * nmemb;
}

static bool initCurlEasy(CURL *&conn, const char *url) {
    CURLcode code;
 
    conn = curl_easy_init();
 
    if (conn == NULL) {
        std::cerr << "Failed to create CURL connection\n";
        exit(EXIT_FAILURE);
    }
 
    code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set error buffer [" << code << "]\n";
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_URL, url);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set URL [" << errorBuffer << "]\n"; 
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set redirect option [" << errorBuffer << "]\n"; 
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set writer [" << errorBuffer << "]\n"; 
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set write data [" << errorBuffer << "]\n"; 
        return false;
    }
 
    return true;
}

// API call to /export/branch_binary_packages/{branch}
// with optional argument `arch`
static nlohmann::json branchBinaryPackages(const std::string &branch, const std::string &arch = "") {
    CURL *conn = NULL;
    CURLcode code;
    std::string url = "https://rdb.altlinux.org/api/export/branch_binary_packages/" + branch;
    if (!arch.empty()) {
        url += "?arch=" + arch;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize CURL connection
    if (!initCurlEasy(conn, url.data())) {
        std::cerr << "Connection initializion failed\n";
        exit(EXIT_FAILURE);
    }

    // Retrieve content for the URL
    code = curl_easy_perform(conn);
    curl_easy_cleanup(conn);
 
    if (code != CURLE_OK) {
        std::cerr << "Failed to get '" << url << "' [ " << errorBuffer << "]\n";
        exit(EXIT_FAILURE);
    }

    nlohmann::json json = nlohmann::json::parse(buffer);
    buffer.clear();

    curl_global_cleanup();

    return json;
}

// Function to convert the result of /export/branch_binary_packages/{branch}
// API call to a dictionary where each architecture maps to a dictionary from
// package names built for this architecture to their respective information.
static std::unordered_map<std::string, std::unordered_map<std::string, nlohmann::json>>
json2Map(const nlohmann::json &data) {
    std::unordered_map<std::string, std::unordered_map<std::string, nlohmann::json>> archToNamesToInfo;
    for (const nlohmann::json &packageInfo : data["packages"]) {
        archToNamesToInfo[packageInfo["arch"]][packageInfo["name"]] = packageInfo;
    }
    return archToNamesToInfo;
}

enum class Cmp {
    LessThan = -1,
    Equal = 0,
    GreaterThan = 1
};

static Cmp CompareVersionParts(const std::string &versionPartStr1,
                               const std::string &versionPartStr2) {
    int versionPartInt1 = -1;
    int versionPartInt2 = -1;

    try {
        versionPartInt1 = std::stoi(versionPartStr1);
        versionPartInt2 = std::stoi(versionPartStr2);
    } catch (const std::invalid_argument &e) {
        // Version parts contain non-numeric characters
        // => compare strings
        if (versionPartStr1 > versionPartStr2) {
            return Cmp::GreaterThan;
        } else if (versionPartStr1 < versionPartStr2) {
            return Cmp::LessThan;
        } else {
            return Cmp::Equal;
        }
    }

    // No exceptions occured => only numeric characters
    // => compare integers
    if (versionPartInt1 > versionPartInt2) {
        return Cmp::GreaterThan;
    } else if (versionPartInt1 < versionPartInt2) {
        return Cmp::LessThan;
    } else {
        return Cmp::Equal;
    }
}

static bool CompareVersionReleaseGT(const std::string &version1, const std::string &release1,
                                    const std::string &version2, const std::string &release2) {
    size_t last1 = 0;
    size_t last2 = 0;
    size_t next1 = 0;
    size_t next2 = 0;

    // Compare every part of Major.Minor.Patch scheme
    while ((next1 = version1.find('.', last1)) != std::string::npos &&
           (next2 = version2.find('.', last2)) != std::string::npos) {
        Cmp cmpResult = CompareVersionParts(version1.substr(last1, next1 - last1),
                                            version2.substr(last2, next2 - last2));

        switch (cmpResult) {
            case Cmp::GreaterThan:
                return true;
            case Cmp::LessThan:
                return false;
        }

        last1 = next1 + 1;
        last2 = next2 + 1;
    }

    // Handle edge case (Patch in the pattern Major.Minor.Patch)
    Cmp cmpResult = CompareVersionParts(version1.substr(last1), version2.substr(last2));

    switch (cmpResult) {
        case Cmp::GreaterThan:
            return true;
        case Cmp::LessThan:
            return false;
    }

    // Versions are equal, compare release info
    if (release1 > release2) {
        return true;
    }

    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage:\n";
        std::cout << argv[0] << " branch1 branch2\n";
        std::cout << "branch1\t- Name of the first branch.\n";
        std::cout << "branch2\t- name of the second branch.\n";
        exit(EXIT_FAILURE);
    }

    std::string branch1(argv[1]);
    std::string branch2(argv[2]);

    nlohmann::json data = branchBinaryPackages(branch1);
    auto b1ArchToNamesToInfo = json2Map(data);

    data = branchBinaryPackages(branch2);
    auto b2ArchToNamesToInfo = json2Map(data);

    data.clear();

    nlohmann::json result;

    // TODO: Move this code into a function to avoid code repetition
    for (const auto &[arch, namesToInfo] : b1ArchToNamesToInfo) {
        // Check if architecture is present in branch1 and not in branch2.
        if (b2ArchToNamesToInfo.find(arch) == b2ArchToNamesToInfo.end()) {
            for (const auto &[pName, pInfo] : namesToInfo) {
                result[arch]["first_not_second"].push_back(pInfo);
            }
            continue;
        }

        for (const auto &[pName, pInfo] : namesToInfo) {
            auto b2It = b2ArchToNamesToInfo[arch].find(pName);
            // Check for packages that are only present in branch1.
            if (b2It == b2ArchToNamesToInfo[arch].end()) {
                result[arch]["first_not_second"].push_back(pInfo);
            } else {
                // Check if version-release in branch1 is greater than in branch2.
                if (CompareVersionReleaseGT(pInfo["version"], pInfo["release"],
                                            b2It->second["version"], b2It->second["release"])) {
                    result[arch]["version-release_greter_first"].push_back(pInfo);
                }
            }
        }
    }

    for (const auto &[arch, namesToInfo] : b2ArchToNamesToInfo) {
        // Check if architecture is present in branch2 and not in branch1.
        if (b1ArchToNamesToInfo.find(arch) == b1ArchToNamesToInfo.end()) {
            for (const auto &[pName, pInfo] : namesToInfo) {
                result[arch]["second_not_first"].push_back(pInfo);
            }
            continue;
        }

        for (const auto &[pName, pInfo] : namesToInfo) {
            // Check for packages that are only present in branch2
            if (b1ArchToNamesToInfo[arch].find(pName) == b1ArchToNamesToInfo[arch].end()) {
                result[arch]["second_not_first"].push_back(pInfo);
            }
        }
    }

    std::ofstream outFile("comparison_" + branch1 + '_' + branch2 + ".json");
    outFile << std::setw(4) << result;
}


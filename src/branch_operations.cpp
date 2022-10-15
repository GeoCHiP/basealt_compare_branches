#include <iostream>
#include <string>
#include <unordered_map>

#include <curl/curl.h>

#include "branch_operations.hpp"

//  libcurl variables for error strings and returned data
static char s_CurlErrorBuffer[CURL_ERROR_SIZE];
static std::string s_CurlBuffer;
 
//  libcurl write callback function
static int CurlWriter(char *data, size_t size, size_t nmemb,
                  std::string *writerData) {
    if (writerData == NULL)
        return 0;

    writerData->append(data, size * nmemb);

    return size * nmemb;
}

static bool CurlEasyInit(CURL *&conn, const char *url) {
    CURLcode code;
 
    conn = curl_easy_init();
 
    if (conn == NULL) {
        std::cerr << "Failed to create CURL connection\n";
        exit(EXIT_FAILURE);
    }
 
    code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, s_CurlErrorBuffer);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set error buffer [" << code << "]\n";
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_URL, url);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set URL [" << s_CurlErrorBuffer << "]\n"; 
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set redirect option [" << s_CurlErrorBuffer << "]\n"; 
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, CurlWriter);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set writer [" << s_CurlErrorBuffer << "]\n"; 
        return false;
    }
 
    code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &s_CurlBuffer);
    if (code != CURLE_OK) {
        std::cerr << "Failed to set write data [" << s_CurlErrorBuffer << "]\n"; 
        return false;
    }
 
    return true;
}

// API call to /export/branch_binary_packages/{branch}
// with optional argument `arch`
nlohmann::json BranchBinaryPackages(const std::string &branch, const std::string &arch) {
    CURL *conn = NULL;
    CURLcode code;
    std::string url = "https://rdb.altlinux.org/api/export/branch_binary_packages/" + branch;
    if (!arch.empty()) {
        url += "?arch=" + arch;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize CURL connection
    if (!CurlEasyInit(conn, url.data())) {
        std::cerr << "Connection initializion failed\n";
        exit(EXIT_FAILURE);
    }

    // Retrieve content for the URL
    code = curl_easy_perform(conn);
    curl_easy_cleanup(conn);
 
    if (code != CURLE_OK) {
        std::cerr << "Failed to get '" << url << "' [ " << s_CurlErrorBuffer << "]\n";
        exit(EXIT_FAILURE);
    }

    nlohmann::json json = nlohmann::json::parse(s_CurlBuffer);
    s_CurlBuffer.clear();

    curl_global_cleanup();

    return json;
}

// Function to convert the result of /export/branch_binary_packages/{branch}
// API call to a dictionary where each architecture maps to a dictionary from
// package names built for this architecture to their respective information.
ArchToNamesToInfo Json2Map(const nlohmann::json &data) {
    ArchToNamesToInfo archToNamesToInfo;
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

void FirstNotSecond(const ArchToNamesToInfo &b1, const ArchToNamesToInfo &b2,
                    const std::string &pLabel, nlohmann::json &o_Result,
                    bool checkVersionRelease, const std::string &vrLabel) {
    for (const auto &[arch, namesToInfo] : b1) {
        // Check if architecture is present in branch1 and not in branch2.
        if (b2.find(arch) == b2.end()) {
            for (const auto &[pName, pInfo] : namesToInfo) {
                o_Result[arch][pLabel].push_back(pInfo);
            }
            continue;
        }

        for (const auto &[pName, pInfo] : namesToInfo) {
            auto b2It = b2.at(arch).find(pName);
            // Check for packages that are only present in branch1.
            if (b2It == b2.at(arch).end()) {
                o_Result[arch][pLabel].push_back(pInfo);
            } else if (checkVersionRelease) {
                // Check if version-release in branch1 is greater than in branch2.
                if (CompareVersionReleaseGT(pInfo["version"], pInfo["release"],
                                            b2It->second["version"], b2It->second["release"])) {
                    o_Result[arch][vrLabel].push_back(pInfo);
                }
            }
        }
    }
}


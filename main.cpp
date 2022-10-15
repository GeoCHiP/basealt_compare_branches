#include <iostream>
#include <string>
#include <fstream>

#include <curl/curl.h>

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

int main() {
    CURL *conn = NULL;
    CURLcode code;
    const char *url = "https://rdb.altlinux.org/api/export/branch_binary_packages/sisyphus";

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Initialize CURL connection
    if (!initCurlEasy(conn, url)) {
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

    std::ofstream outFile("sisyphus_packages.json");
    outFile << buffer;

    curl_global_cleanup();
}


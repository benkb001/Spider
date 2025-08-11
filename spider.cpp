#include <algorithm>
#include <curl/curl.h> 
#include <iostream> 
#include <iterator> 
#include <regex>
#include <string> 
#include <unordered_map>
#include <vector>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb; 
    ((std::string*)userp)->append((char*)contents, total);
    return total;
}

int numMatches(std::string& str, std::regex& r) {
    auto begin = std::sregex_iterator(str.begin(), str.end(), r); 
    auto end = std::sregex_iterator(); 
    return std::distance(begin, end); 
}

bool containsKey(std::string& key, std::unordered_map<std::string, int>& map) {
    return map.find(key) != map.end(); 
}

void traverseSite(std::string& url, std::string& readBuffer, std::unordered_map<std::string, int>& visitedSites, 
    CURLcode& res, CURL* curl, std::regex& tokenRegex, std::regex& linkRegex, int& limit) {
    
    readBuffer.clear();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 
    res = curl_easy_perform(curl); 
    
    if(res != CURLE_OK) {
        visitedSites[url] = -1; 
        std::cout << "failed\n"; 
    } else {
        visitedSites[url] = numMatches(readBuffer, tokenRegex); 
        auto it = readBuffer.cbegin(); 
        auto end = readBuffer.cend(); 
        std::smatch match; 
        while (std::regex_search(it, end, match, linkRegex)) {
            std::string link = match[1]; 
            if (!(containsKey(link, visitedSites)) && (visitedSites.size() < limit)) {
                traverseSite(link, readBuffer, visitedSites, res, curl, tokenRegex, linkRegex, limit); 
            }
            it = match.suffix().first; 
        }
    }
}

int main() {
    std::string readBuffer; 
    CURLcode res; 
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init(); 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); 

    std::unordered_map<std::string, int> visitedSites;
    std::vector<std::string> tokens; 
    std::string tokenRegexStr = ""; 

    for (size_t i = 0; i < tokens.size(); i++) {
        tokenRegexStr += tokens[i];
        if (i != tokens.size() - 1) {
            tokenRegexStr += "|";
        }
    }

    std::regex tokenRegex(tokenRegexStr);
    std::regex linkRegex("a href=\"(.*?)\""); 
    std::string urlInit;
    int limit = 100; 
    
    traverseSite(urlInit, readBuffer, visitedSites, res, curl, tokenRegex, linkRegex, limit); 

    for (const auto& pair : visitedSites) {
        std::cout << "Key: " << pair.first << ", Value: " << pair.second << "\n";
    }

    if(curl) {
        curl_easy_cleanup(curl); 
    }

    curl_global_cleanup(); 
    return 0; 
}
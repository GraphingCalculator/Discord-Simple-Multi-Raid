#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <chrono>
#include <curl/curl.h>

std::vector<std::string> tokens;

std::mutex tokensMutex;

void readTokensFromFile() {
    std::ifstream file("tokens.txt");
    if (!file) {
        std::ofstream newFile("tokens.txt");
        newFile.close();
    } else {
        std::string line;
        while (std::getline(file, line)) {
            tokens.push_back(line);
        }
        file.close();
    }
}

void writeToTokensFile(const std::string& token) {
    std::lock_guard<std::mutex> lock(tokensMutex);
    std::ofstream file("tokens.txt", std::ios::app);
    if (file) {
        file << token << '\n';
    }
}

std::string getRandomToken() {
    std::lock_guard<std::mutex> lock(tokensMutex);
    if (tokens.empty()) {
        return "";
    }
    int randomIndex = rand() % tokens.size();
    return tokens[randomIndex];
}

void setup(std::string& serverId, std::string& msg) {
    std::cout << "Enter your serverID below: ";
    std::cin >> serverId;

    std::cout << "Enter your message below: ";
    std::cin.ignore();
    std::getline(std::cin, msg);
}

void mainLoop(const std::string& serverId, const std::string& msg) {
    while (true) {
        std::string token = getRandomToken();
        if (token.empty()) {
            std::cout << "No tokens available. Exiting..." << std::endl;
            return;
        }

        CURL* curl = curl_easy_init();
        if (curl) {
            std::string url = "https://discord.com/api/v8/channels/" + serverId + "/messages";
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, ("authorization: " + token).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string postData = "{\"content\":\"" + msg + "\"}";
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

            CURLcode res = curl_easy_perform(curl);
            long responseCode;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            std::cout << responseCode << std::endl;

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (responseCode == 401) {
                // Token is invalid remove it from list and try again
                tokensMutex.lock();
                tokens.erase(std::remove(tokens.begin(), tokens.end(), token), tokens.end());
                tokensMutex.unlock();
            } else {
                break;
            }
        }
    }
}

int main() {
    readTokensFromFile();

    std::string serverId, msg;
    setup(serverId, msg);

    int numThreads = 60;
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(mainLoop, serverId, msg);
    }

    for (int i = 0; i < numThreads; ++i) {
        threads[i].join();
    }

    return 0;
}

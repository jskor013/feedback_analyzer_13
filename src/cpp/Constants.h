#pragma once
#include <string>
#include <vector>
#include <map>

class Constants {
public:
    static std::map<std::string, std::vector<std::string>> SENTIMENT_KEYWORDS;
    static std::map<std::string, std::map<std::string, std::vector<std::string>>> CATEGORY_KEYWORDS;

    static void init();
};

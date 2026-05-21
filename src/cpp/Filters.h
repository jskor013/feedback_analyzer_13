#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "Feedback.h"
#include "Constants.h"

class Filters {
private:
    static std::map<std::string, std::vector<std::string>> S_KEYWORDS;

    static bool containsAny(const std::string& text, const std::vector<std::string>& keywords) {
        for (const auto& kw : keywords) {
            if (text.find(kw) != std::string::npos) return true;
        }
        return false;
    }

public:
    static void initFilterKeywords();

    std::vector<Feedback> fil(const std::vector<Feedback>& dataList,
                              const std::string& sFilter,
                              const std::string& kFilter) {
        std::vector<Feedback> tmpFiltered;

        if (sFilter != u8"전체") {
            for (const auto& item : dataList) {
                std::string txt = item.getText();
                std::string currentSentiment = u8"중립";

                if (containsAny(txt, S_KEYWORDS[u8"긍정"])) {
                    currentSentiment = u8"긍정";
                } else if (containsAny(txt, S_KEYWORDS[u8"부정"])) {
                    currentSentiment = u8"부정";
                } else if (containsAny(txt, S_KEYWORDS[u8"중립"])) {
                    currentSentiment = u8"중립";
                }

                if (currentSentiment == sFilter) {
                    tmpFiltered.push_back(item);
                }
            }
        } else {
            tmpFiltered = dataList;
        }

        std::vector<Feedback> finalFiltered;
        if (kFilter != u8"전체") {
            for (const auto& item : tmpFiltered) {
                std::string txt = item.getText();
                if (Constants::CATEGORY_KEYWORDS.count(kFilter)) {
                    const auto& catMap = Constants::CATEGORY_KEYWORDS[kFilter];
                    for (const auto& subEntry : catMap) {
                        if (subEntry.first == "main") continue;
                        if (containsAny(txt, subEntry.second)) {
                            finalFiltered.push_back(item);
                            break;
                        }
                    }
                }
            }
        } else {
            finalFiltered = tmpFiltered;
        }

        for (const auto& i : finalFiltered) {
            std::cout << i.getText() << std::endl;
        }

        return finalFiltered;
    }
};

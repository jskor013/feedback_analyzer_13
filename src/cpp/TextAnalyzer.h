#pragma once
#include <string>
#include <vector>
#include <map>
#include "Feedback.h"
#include "Constants.h"

class TextAnalyzer {
private:
    static std::map<std::string, int> globalSent;
    static std::map<std::string, int> globalKw;

    static bool containsAny(const std::string& text, const std::vector<std::string>& keywords) {
        for (const auto& kw : keywords) {
            if (text.find(kw) != std::string::npos) return true;
        }
        return false;
    }

public:
    std::map<std::string, int> sent(const std::vector<Feedback>& feedbacks) {
        std::map<std::string, int> res;
        res[u8"긍정"] = 0;
        res[u8"중립"] = 0;
        res[u8"부정"] = 0;

        for (const auto& f : feedbacks) {
            std::string txt = f.getText();
            std::string s = u8"중립";
            if (containsAny(txt, Constants::SENTIMENT_KEYWORDS[u8"긍정"])) {
                s = u8"긍정";
            } else if (containsAny(txt, Constants::SENTIMENT_KEYWORDS[u8"부정"])) {
                s = u8"부정";
            }
            res[s]++;
        }

        globalSent = res;
        return res;
    }

    std::map<std::string, int> kw(const std::vector<Feedback>& feedbacks) {
        std::map<std::string, int> res2;
        for (const auto& entry : Constants::CATEGORY_KEYWORDS) {
            res2[entry.first] = 0;
        }

        for (const auto& f : feedbacks) {
            std::string txt = f.getText();
            for (const auto& entry : Constants::CATEGORY_KEYWORDS) {
                const std::string& cat = entry.first;
                if (entry.second.count("main")) {
                    const auto& kws = entry.second.at("main");
                    if (containsAny(txt, kws)) {
                        res2[cat]++;
                    }
                }
            }
        }

        globalKw = res2;
        return res2;
    }
};

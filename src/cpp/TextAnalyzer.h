#pragma once
#include <string>
#include <vector>
#include <map>
#include "Feedback.h"
#include "Constants.h"
#include "FeedbackClassifier.h"

class TextAnalyzer {
public:
    std::map<std::string, int> sent(const std::vector<Feedback>& feedbacks) {
        std::map<std::string, int> res;
        res[u8"긍정"] = 0;
        res[u8"중립"] = 0;
        res[u8"부정"] = 0;

        for (const auto& f : feedbacks) {
            res[FeedbackClassifier::classifySentiment(f)]++;
        }

        return res;
    }

    std::map<std::string, int> kw(const std::vector<Feedback>& feedbacks) {
        std::map<std::string, int> res2;
        for (const auto& entry : Constants::CATEGORY_KEYWORDS) {
            res2[entry.first] = 0;
        }

        for (const auto& f : feedbacks) {
            for (const auto& entry : Constants::CATEGORY_KEYWORDS) {
                const std::string& cat = entry.first;
                if (FeedbackClassifier::matchesCategory(f, cat)) {
                    res2[cat]++;
                }
            }
        }

        return res2;
    }
};

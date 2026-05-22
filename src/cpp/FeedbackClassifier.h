#pragma once

#include "Constants.h"
#include "Feedback.h"

#include <string>
#include <vector>

class FeedbackClassifier {
public:
    static bool containsAny(const std::string& text, const std::vector<std::string>& keywords) {
        for (const auto& keyword : keywords) {
            if (text.find(keyword) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    static std::string classifySentiment(const Feedback& feedback) {
        const std::string& text = feedback.getText();
        if (containsAny(text, Constants::SENTIMENT_KEYWORDS[u8"긍정"])) {
            return u8"긍정";
        }
        if (containsAny(text, Constants::SENTIMENT_KEYWORDS[u8"부정"])) {
            return u8"부정";
        }
        return u8"중립";
    }

    static bool matchesSentiment(const Feedback& feedback, const std::string& sentiment) {
        return sentiment == u8"전체" || classifySentiment(feedback) == sentiment;
    }

    static bool matchesCategory(const Feedback& feedback, const std::string& category) {
        if (category == u8"전체") {
            return true;
        }

        const auto categoryIt = Constants::CATEGORY_KEYWORDS.find(category);
        if (categoryIt == Constants::CATEGORY_KEYWORDS.end()) {
            return false;
        }

        const std::string& text = feedback.getText();
        for (const auto& keywordGroup : categoryIt->second) {
            if (containsAny(text, keywordGroup.second)) {
                return true;
            }
        }
        return false;
    }
};

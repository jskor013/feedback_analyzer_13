#pragma once
#include <string>
#include <vector>
#include <map>
#include "Feedback.h"
#include "FeedbackClassifier.h"

class Filters {
public:
    static void initFilterKeywords();

    std::vector<Feedback> filterFeedbacks(const std::vector<Feedback>& dataList,
                                          const std::string& sFilter,
                                          const std::string& kFilter) {
        std::vector<Feedback> filtered;
        for (const auto& item : dataList) {
            if (FeedbackClassifier::matchesSentiment(item, sFilter) &&
                FeedbackClassifier::matchesCategory(item, kFilter)) {
                filtered.push_back(item);
            }
        }
        return filtered;
    }

    std::vector<Feedback> fil(const std::vector<Feedback>& dataList,
                              const std::string& sFilter,
                              const std::string& kFilter) {
        return filterFeedbacks(dataList, sFilter, kFilter);
    }
};

#pragma once
#include <vector>
#include <map>
#include <string>
#include "Feedback.h"

class Session {
private:
    static std::vector<Feedback> currentFeedbacks;
    static std::map<std::string, std::string> internalData;
    static std::map<std::string, std::string> filterOptions;

public:
    static void initSessionStateUgly() {
        // already initialized as static
    }

    static std::vector<Feedback>& getOldDataFromSession(const std::string& key) {
        return currentFeedbacks;
    }

    static void updateCurrentFeedbacks(const std::vector<Feedback>& feedbacks) {
        currentFeedbacks = feedbacks;
    }

    static std::vector<Feedback>& getCurrentFeedbacks() {
        return currentFeedbacks;
    }
};

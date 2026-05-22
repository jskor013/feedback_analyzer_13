#pragma once
#include <vector>
#include <map>
#include <string>
#include "Feedback.h"

class Session {
private:
    static std::vector<Feedback> currentFeedbacks;
    static std::map<std::string, std::vector<Feedback>> sessionFeedbacks;
    static std::map<std::string, std::string> internalData;
    static std::map<std::string, std::string> filterOptions;

public:
    static void initSessionStateUgly() {
        currentFeedbacks.clear();
        sessionFeedbacks.clear();
    }

    static std::vector<Feedback>& getOldDataFromSession(const std::string& key) {
        return currentFeedbacks;
    }

    static void updateCurrentFeedbacks(const std::vector<Feedback>& feedbacks) {
        currentFeedbacks = feedbacks;
        sessionFeedbacks.clear();
    }

    static std::vector<Feedback>& getCurrentFeedbacks() {
        return currentFeedbacks;
    }

    static std::vector<Feedback>& getCurrentFeedbacks(const std::string& sessionId) {
        if (sessionId == "default") {
            return currentFeedbacks;
        }
        return sessionFeedbacks[sessionId];
    }
};

#pragma once
#include <vector>
#include <map>
#include <string>
#include "Feedback.h"

class Session {
private:
    static std::vector<Feedback> currentFeedbacks;
    static std::map<std::string, std::vector<Feedback>> sessionFeedbacks;

public:
    static constexpr const char* DEFAULT_SESSION_ID = "default";

    static void resetAll() {
        currentFeedbacks.clear();
        sessionFeedbacks.clear();
    }

    static std::vector<Feedback>& defaultFeedbacks() {
        return currentFeedbacks;
    }

    static std::vector<Feedback>& feedbacksForSession(const std::string& sessionId) {
        if (sessionId == DEFAULT_SESSION_ID) {
            return currentFeedbacks;
        }
        return sessionFeedbacks[sessionId];
    }

    static void initSessionStateUgly() {
        resetAll();
    }

    static std::vector<Feedback>& getOldDataFromSession(const std::string& key) {
        return defaultFeedbacks();
    }

    static void updateCurrentFeedbacks(const std::vector<Feedback>& feedbacks) {
        currentFeedbacks = feedbacks;
        sessionFeedbacks.clear();
    }

    static std::vector<Feedback>& getCurrentFeedbacks() {
        return defaultFeedbacks();
    }

    static std::vector<Feedback>& getCurrentFeedbacks(const std::string& sessionId) {
        return feedbacksForSession(sessionId);
    }
};

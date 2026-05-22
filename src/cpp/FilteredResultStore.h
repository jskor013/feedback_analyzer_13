#pragma once

#include "Feedback.h"

#include <map>
#include <string>
#include <vector>

class FilteredResultStore {
public:
    static std::map<std::string, std::vector<Feedback>>& all() {
        static std::map<std::string, std::vector<Feedback>> results;
        return results;
    }

    static void save(const std::string& sessionId, const std::vector<Feedback>& feedbacks) {
        all()[sessionId] = feedbacks;
    }

    static void clear(const std::string& sessionId) {
        all().erase(sessionId);
    }

    static const std::vector<Feedback>* find(const std::string& sessionId) {
        const auto it = all().find(sessionId);
        if (it == all().end() || it->second.empty()) {
            return nullptr;
        }
        return &it->second;
    }
};

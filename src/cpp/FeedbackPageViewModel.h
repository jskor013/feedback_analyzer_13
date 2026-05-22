#pragma once

#include "Feedback.h"

#include <map>
#include <string>
#include <vector>

struct FeedbackPageViewModel {
    std::string success;
    std::string warning;
    std::string error;
    std::map<std::string, int> sentimentResults;
    std::map<std::string, int> keywordResults;
    std::vector<Feedback> feedbacks;
};

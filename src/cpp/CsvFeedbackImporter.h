#pragma once

#include "Feedback.h"
#include "FileHandler.h"

#include <algorithm>
#include <string>
#include <vector>

struct CsvImportResult {
    std::vector<Feedback> feedbacks;

    bool hasFeedbacks() const {
        return !feedbacks.empty();
    }
};

class CsvFeedbackImporter {
public:
    static CsvImportResult importFeedbacks(const std::string& content) {
        CsvImportResult result;
        if (content.empty()) {
            return result;
        }

        bool firstLine = true;
        size_t textColumn = 0;
        for (const auto& fields : FileHandler::parseCsvRecords(content)) {
            if (fields.empty() || (fields.size() == 1 && fields[0].empty())) {
                continue;
            }

            if (firstLine) {
                firstLine = false;
                const auto header = std::find(fields.begin(), fields.end(), "text");
                if (header != fields.end()) {
                    textColumn = static_cast<size_t>(std::distance(fields.begin(), header));
                    continue;
                }
            }

            if (fields.size() > textColumn && !fields[textColumn].empty()) {
                result.feedbacks.push_back(Feedback(fields[textColumn]));
            }
        }
        return result;
    }
};

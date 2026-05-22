#pragma once

#include "Feedback.h"
#include "FileHandler.h"

#include <sstream>
#include <string>
#include <vector>

class CsvFeedbackExporter {
public:
    static std::string exportFeedbacks(const std::vector<Feedback>& feedbacks) {
        std::ostringstream csv;
        csv << "\xEF\xBB\xBF";
        csv << "text\n";
        for (const auto& feedback : feedbacks) {
            csv << FileHandler::escapeCsvField(feedback.getText()) << "\n";
        }
        return csv.str();
    }
};

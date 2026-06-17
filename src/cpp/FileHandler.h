#pragma once
#include <string>
#include <vector>

class FileHandler {
public:
    static std::vector<std::vector<std::string>> parseCsvRecords(const std::string& content) {
        std::vector<std::vector<std::string>> records;
        std::vector<std::string> record;
        std::string field;
        bool inQuotes = false;

        for (size_t i = 0; i < content.size(); ++i) {
            const char c = content[i];
            if (c == '"') {
                if (inQuotes && i + 1 < content.size() && content[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                record.push_back(field);
                field.clear();
            } else if ((c == '\n' || c == '\r') && !inQuotes) {
                if (c == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
                    ++i;
                }
                record.push_back(field);
                records.push_back(record);
                record.clear();
                field.clear();
            } else {
                field += c;
            }
        }

        if (!field.empty() || !record.empty()) {
            record.push_back(field);
            records.push_back(record);
        }
        return records;
    }

    static std::string escapeCsvField(const std::string& field) {
        if (field.find_first_of(",\"\r\n") == std::string::npos) {
            return field;
        }

        std::string escaped = "\"";
        for (const char c : field) {
            if (c == '"') {
                escaped += "\"\"";
            } else {
                escaped += c;
            }
        }
        escaped += '"';
        return escaped;
    }
};

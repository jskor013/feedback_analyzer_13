#pragma once

#include "LogEvent.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

class ConsoleLogSink {
public:
    static void write(const LogEvent& event) {
        std::ostream& stream = event.level == LogLevel::Error ? std::cerr : std::cout;
        stream << "[" << getTimestamp() << "] " << levelName(event.level) << ": " << event.message << std::endl;
    }

private:
    static std::string getTimestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    static const char* levelName(LogLevel level) {
        switch (level) {
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Debug: return "DEBUG";
        }
        return "INFO";
    }
};

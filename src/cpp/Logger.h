#pragma once
#include "ConsoleLogSink.h"

#include <string>

class Logger {
private:
    static bool debugMode;

    static void log(const LogEvent& event) {
        ConsoleLogSink::write(event);
    }

public:
    static void logInfo(const std::string& message) {
        log({LogLevel::Info, message});
    }

    static void logWarning(const std::string& message) {
        log({LogLevel::Warning, message});
    }

    static void logError(const std::string& message) {
        log({LogLevel::Error, message});
    }

    static void logDebug(const std::string& message) {
        if (debugMode) {
            log({LogLevel::Debug, message});
        }
    }

    static void setDebugMode(bool mode) { debugMode = mode; }
    static bool isDebugMode() { return debugMode; }
};

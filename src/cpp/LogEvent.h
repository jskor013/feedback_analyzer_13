#pragma once

#include <string>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

struct LogEvent {
    LogLevel level;
    std::string message;
    std::string event;
    int count = -1;
    std::string errorReason;
    std::string requestScope;
};

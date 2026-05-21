#pragma once

#include <string>
#include <utility>

class Feedback {
    std::string text;

public:
    explicit Feedback(std::string t) : text(std::move(t)) {}

    const std::string& getText() const { return text; }
};

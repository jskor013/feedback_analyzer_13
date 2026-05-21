#pragma once
#include <string>
#include <vector>

class UIComponents {
private:
    static const std::vector<std::string> CATS;

public:
    static const std::vector<std::string>& getCategories() {
        return CATS;
    }
};

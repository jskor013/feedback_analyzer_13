#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Feedback.h"

class FileHandler {
public:
    void saveResult(const std::vector<Feedback>& data) {
        std::cout << "saveResult" << data.size() << std::endl;
        for (const auto& iter : data)
            std::cout << iter.getText() << std::endl;
    }

    void save(const std::vector<Feedback>& data) {
        saveResult(data);
    }
};

//
// Created by Marat on 18.08.25.
//

#pragma once
#include <string>
#include <vector>

struct FileStats {
    std::string name;
    size_t lines;
};

struct LangConfig {
    std::vector<std::string> extension;
    std::string name;
};
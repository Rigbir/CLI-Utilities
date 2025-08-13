//
// Created by Marat on 13.08.25.
//

#pragma once
#include <vector>
#include <string>
#include "Colors.h"

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(const std::vector<std::string>& args) = 0;
};
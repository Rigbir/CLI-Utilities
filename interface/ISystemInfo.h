//
// Created by Marat on 19.08.25.
//

#pragma once
#include "Command.h"

class ISystemInfo : public Command {
public:
    virtual std::vector<std::string> getCPUUsage() = 0;
    virtual std::vector<std::string> getRAMUsage() = 0;
    virtual std::vector<std::string> getDiskUsage() = 0;
};

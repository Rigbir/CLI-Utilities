//
// Created by Marat on 19.08.25.
//

#pragma once
#include "Command.h"

class ISystemInfo : public Command {
public:
    virtual void getSystemInfo() = 0;
    virtual void getCPUUsage() = 0;
    virtual void getRAMUsage() = 0;
    virtual void getDiskUsage() = 0;
    virtual void getTemperature() = 0;
};

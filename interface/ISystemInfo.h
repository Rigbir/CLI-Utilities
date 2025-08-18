//
// Created by Marat on 19.08.25.
//

#pragma once
#include "Command.h"

class ISystemInfo : public Command {
public:
    virtual void getSystemInfo() const = 0;
    virtual void getCPUUsage() const = 0;
    virtual void getRAMUsage() const = 0;
    virtual void getDiskUsage() const = 0;
    virtual void getTemperature() const = 0;
};

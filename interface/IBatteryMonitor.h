//
// Created by Marat on 13.08.25.
//

#pragma once
#include "Command.h"
#include <CoreFoundation/CoreFoundation.h>

class IBatteryMonitor: public Command {
public:
    virtual bool isCharging() const = 0;
    virtual int getRegistryIntValue(CFStringRef key) const = 0;
    virtual int getCapacity() const = 0;
    virtual int getTimeRemaining() const = 0;
    virtual int getCycleCount() const = 0;
    virtual double getHealth() const = 0;
};

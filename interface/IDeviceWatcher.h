//
// Created by Marat on 13.08.25.
//

#pragma once
#include "Command.h"

class IDeviceWatcher : public Command {
public:
    virtual void usbRun() const = 0;
    virtual void audioRun() const = 0;
    virtual void displayRun() const = 0;
};

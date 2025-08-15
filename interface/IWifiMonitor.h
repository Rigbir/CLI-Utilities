//
// Created by Marat on 13.08.25.
//

#pragma once
#include "Command.h"

class IWifiMonitor : public Command {
public:
    virtual void showCurrentConnections() const = 0;
    virtual void listConnections() const = 0;
    virtual void monitorConnections() const = 0;
};

//
// Created by Marat on 13.08.25.
//

#pragma once
#include "Command.h"

class ICleaner : public Command {
public:
    virtual void getAllInfo() = 0;
    virtual std::string getStats(const std::string& path) const = 0;
    virtual void removeFile()  = 0;
};

//
// Created by Marat on 17.08.25.
//

#pragma once
#include "Command.h"

class ICodeCounter : public Command {
public:
    virtual void getFolderStats() const = 0;
    virtual void getLangStats() = 0;
};

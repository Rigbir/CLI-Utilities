//
// Created by Marat on 17.08.25.
//

#pragma once
#include "ICodeCounter.h"

class CodeCounter final : public ICodeCounter {
public:
    void execute(const std::vector<std::string>& args) override;
    void getFolderStats() const override;
};

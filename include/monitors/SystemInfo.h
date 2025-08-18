//
// Created by Marat on 19.08.25.
//

#pragma once
#include "ISystemInfo.h"

class SystemInfo final : public ISystemInfo {
public:
    void execute(const std::vector<std::string>& args) override;
    void getSystemInfo() const override;
    void getCPUUsage() const override;
    void getRAMUsage() const override;
    void getDiskUsage() const override;
    void getTemperature() const override;

private:

};

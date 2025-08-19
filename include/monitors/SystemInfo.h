//
// Created by Marat on 19.08.25.
//

#pragma once
#include "ISystemInfo.h"
#include <tuple>
#include <mach/host_info.h>

class SystemInfo final : public ISystemInfo {
public:
    void execute(const std::vector<std::string>& args) override;
    void getSystemInfo() override;
    void getCPUUsage() override;
    void getRAMUsage() override;
    void getDiskUsage() override;
    void getTemperature() override;

private:
    void runLiveMonitoring();

    host_cpu_load_info_data_t prev = {};
    std::tuple<double, double, double> CPUUsageCalculation(host_cpu_load_info_data_t& curr);

};

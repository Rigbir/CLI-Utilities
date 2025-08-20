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
    std::vector<std::string> getCPUUsage() override;
    std::vector<std::string> getRAMUsage() override;
    std::vector<std::string> getDiskUsage() override;

private:
    void runLiveMonitoring();

    static double bytesToMb(double bytes);
    static double bytesToGb(double bytes);
    static double bytesToGB(double bytes);
    static double percent(double val, double total);
    static std::string toString(double val);
    static std::string getMacVersion(const char* name);
    static std::string getMacName();

    std::string productName;
    std::string productVersion;
    host_cpu_load_info_data_t prev = {};
    std::tuple<double, double, double> CPUUsageCalculation(host_cpu_load_info_data_t& curr);
};

//
// Created by Marat on 13.08.25.
//

#pragma once
#include <iostream>
#include "IBatteryMonitor.h"

class BatteryMonitor final : public IBatteryMonitor {
public:
    void execute(const std::vector<std::string>& args) override;
    [[nodiscard]] bool isCharging() const override;
    [[nodiscard]] int getRegistryIntValue(CFStringRef key) const override;
    [[nodiscard]] int getCapacity() const override;
    [[nodiscard]] int getCycleCount() const override;
    [[nodiscard]] int getTimeRemaining() const override;
    [[nodiscard]] double getHealth() const override;
    std::pair<std::string, std::string> animatedBattery(int batteryPercent) const;

private:
    static int getIntValue(CFStringRef key);
};

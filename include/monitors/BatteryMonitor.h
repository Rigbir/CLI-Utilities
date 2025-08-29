//
// Created by Marat on 13.08.25.
//

#pragma once
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

    [[nodiscard]] static std::pair<std::string, std::string> animatedBar(int valuePercent,
                                                                         const std::vector<std::string> &pulseChars,
                                                                         bool animate);

    [[nodiscard]] std::pair<std::string, std::string> animatedBattery(int batteryPercent) const;
    [[nodiscard]] std::pair<std::string, std::string> animatedCycleCount(int cycleCount) const;
    [[nodiscard]] std::pair<std::string, std::string> animatedHealth(double healthPercent) const;
    [[nodiscard]] std::pair<std::string, std::string> animatedTime(int time) const;
    [[nodiscard]] std::pair<std::string, std::string> staticAnimation(int value) const;

private:
    [[nodiscard]] static int getIntValue(CFStringRef key);
    void runLiveMonitor() const;
    void printStatus(bool animated) const;
    void animatedStatus() const;
    void staticStatus() const;
};

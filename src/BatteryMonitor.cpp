//
// Created by Marat on 13.08.25.
//

#include "BatteryMonitor.h"
#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

void BatteryMonitor::printStatus(const bool animated) const {
    clearScreen();

    std::string color, bar;
    std::string colorCycle, barCycle;
    std::string colorHealth, barHealth;
    std::string colorTime, barTime;

    if (animated) {
        std::tie(color, bar) = animatedBattery(getCapacity());
        std::tie(colorCycle, barCycle) = animatedCycleCount(getCycleCount());
        std::tie(colorHealth, barHealth) = animatedHealth(getHealth());
        std::tie(colorTime, barTime) = animatedTime(getTimeRemaining());
    } else {
        std::tie(color, bar) = staticAnimation(getCapacity());
        std::tie(colorCycle, barCycle) = staticAnimation(getCycleCount());
        std::tie(colorHealth, barHealth) = staticAnimation(getHealth());
        std::tie(colorTime, barTime) = staticAnimation(getTimeRemaining());
    }

    std::cout << '\n';
    std::cout << colorText(BWhite, "Battery: ")
              << colorText(C_White, std::to_string(getCapacity()) + "% [" + color + bar + "] ")
              << (isCharging() ? colorText(BGreen, "(charging)") : colorText(BRed, "(discharging)")) << "\n\n";

    const int percentCycle = 100 - (getCycleCount() / 10);
    std::cout << colorText(BPurple, "Cycle Count: ")
              << colorText(C_Purple, std::to_string(percentCycle) + "%"
                            + " (" + std::to_string(getCycleCount()) + ")"
                            + " [" + colorCycle + barCycle + "] ") << "\n\n";

    double health = getHealth();
    std::cout << colorText(BCyan, "Health: ")
              << C_Cyan << std::fixed << std::setprecision(2) << health << "%" << C_RESET
              << colorText(C_Cyan, " [" + colorHealth + barHealth + "] ") << "\n\n";

    const int time = getTimeRemaining();
    std::cout << colorText(BBlue, "Time remaining: ")
              << colorText(C_Blue, (time > 0 ? std::to_string(time) + " minutes" : "calculating..."))
              << colorText(C_Blue, " [" + colorTime + barTime + "] ") << '\n';

    if (animated) {
        std::cout << colorText(BWhite, "\nPress 'q' + Enter to go back.\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}

void BatteryMonitor::animatedStatus() const {
    printStatus(true);
}

void BatteryMonitor::staticStatus() const {
    printStatus(false);
}

void BatteryMonitor::runLiveMonitor() const {
    std::atomic<bool> stopFlag = false;
    std::thread inputThread([&stopFlag]() {
        std::string line;
        while (!stopFlag) {
            std::getline(std::cin, line);
            if (line == "q" || line == "quit") stopFlag = true;
        }
    });

    while (!stopFlag) {
        animatedStatus();
    }

    inputThread.join();
}

void BatteryMonitor::execute(const std::vector<std::string>& args) {
    (void) args;
    clearScreen();
    for (size_t i = 0; i < 9; ++i) std::cout << '\n';

    const std::vector<std::string> batteryInfo = {
        "Battery Monitoring Utility",
        "",
        "Commands:",
        "  1  - Show battery status (animated)",
        "  2  - Show battery status (static)",
        "",
        "Navigation:",
        "  q, quit - go back to main menu"
    };
    printBox(batteryInfo);

    std::string input;
    while (true) {
        std::cout << "\n\n" << colorText(BWhite, centered("Enter command: ", termWidth()));
        std::cin >> input;

        if (input == "q" || input == "quit") return;

        switch (std::stoi(input)) {
            case 1: runLiveMonitor(); break;
            case 2: staticStatus(); break;
            default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
        }
    }
}

int BatteryMonitor::getIntValue(const CFStringRef key) {
    const CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    const CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

    int value = -1;
    if (CFArrayGetCount(sources) > 0) {
        const CFDictionaryRef desc = IOPSGetPowerSourceDescription(
            blob,
            CFArrayGetValueAtIndex(sources, 0)
        );

        auto num = static_cast<CFNumberRef>(CFDictionaryGetValue(desc, key));
        if (num) CFNumberGetValue(num, kCFNumberIntType, &value);
    }

    CFRelease(blob);
    CFRelease(sources);
    return value;
}

bool BatteryMonitor::isCharging() const {
    const CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    const CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

    bool value = false;
    if (CFArrayGetCount(sources) > 0) {
        const CFDictionaryRef desc = IOPSGetPowerSourceDescription(
            blob,
            CFArrayGetValueAtIndex(sources, 0)
        );
        auto b = static_cast<CFBooleanRef>(CFDictionaryGetValue(desc, CFSTR(kIOPSIsChargingKey)));
        if (b) value = CFBooleanGetValue(b);
    }

    CFRelease(blob);
    CFRelease(sources);
    return value;
}

int BatteryMonitor::getRegistryIntValue(const CFStringRef key) const {
    const io_service_t battery = IOServiceGetMatchingService(
                                kIOMainPortDefault,
                                IOServiceMatching("AppleSmartBattery"));
    if (!battery) return -1;

    const CFTypeRef val = IORegistryEntryCreateCFProperty(battery, key, kCFAllocatorDefault, 0);
    IOObjectRelease(battery);

    int result = -1;
    if (val && CFGetTypeID(val) == CFNumberGetTypeID()) {
        CFNumberGetValue(static_cast<CFNumberRef>(val), kCFNumberIntType, &result);
    }

    CFRelease(val);
    return result;
}

int BatteryMonitor::getCapacity() const {
    return getIntValue(CFSTR(kIOPSCurrentCapacityKey));
}

int BatteryMonitor::getCycleCount() const {
    return getRegistryIntValue(CFSTR("CycleCount"));
}

double BatteryMonitor::getHealth() const {
    const int designCap = getRegistryIntValue(CFSTR("DesignCapacity"));
    const int nominalCap = getRegistryIntValue(CFSTR("NominalChargeCapacity"));
    return (designCap > 0) ? (static_cast<double>(nominalCap) / designCap) * 100 : -1;
}

int BatteryMonitor::getTimeRemaining() const {
    const CFStringRef key = isCharging() ? CFSTR(kIOPSTimeToFullChargeKey) : CFSTR(kIOPSTimeToEmptyKey);
    return getIntValue(key);
}

std::pair<std::string, std::string> BatteryMonitor::animatedBar(const int valuePercent,
                                                                const std::vector<std::string>& pulseChars = {"█"},
                                                                const bool animate = false) const {
    constexpr int width = 20;
    static int animationStep = 0;

    std::string color;
    if (valuePercent > 80) color = BGreen;
    else if (valuePercent >= 40) color = BYellow;
    else color = BRed;

    const int filled = valuePercent * width / 100;
    const int empty = width - filled;

    std::vector<std::string> bar(filled, "█");
    bar.insert(bar.end(), empty, "-");

    if (animate && filled > 0) {
        for (int i = 0; i < filled; ++i) {
            bar[i] = pulseChars[(i + animationStep) % pulseChars.size()];
        }
        ++animationStep;
    }

    std::string barStr;
    for (const auto& c : bar) {
        barStr += c;
    }

    return {color, barStr};
}

std::pair<std::string, std::string> BatteryMonitor::animatedBattery(const int batteryPercent) const {
    std::vector<std::string> pulseChars = {"█", "░"};
    return isCharging() ? animatedBar(batteryPercent, pulseChars, true)
                        : animatedBar(batteryPercent);

}

std::pair<std::string, std::string> BatteryMonitor::animatedCycleCount(const int cycleCount) const {
    std::vector<std::string> pulse = {"█","▓","▒","░"};
    int cycleCountPercent = 100 - (cycleCount / 10);
    return animatedBar(cycleCountPercent, pulse, true);
}

std::pair<std::string, std::string> BatteryMonitor::animatedHealth(const double healthPercent) const {
    std::vector<std::string> pulse = {"█", " "};
    return animatedBar(static_cast<int>(healthPercent), pulse, true);
}

std::pair<std::string, std::string> BatteryMonitor::animatedTime(const int time) const {
    static int timeToCharge = 120;
    static int timeToDischarge = 500;
    const int timePercent = isCharging() ? time * 100 / timeToCharge
                                         : time * 100 / timeToDischarge;
    std::vector<std::string> colors = {
        std::string(BWhite) + "█",
        std::string(BRed) + "█",
        std::string(BGreen) + "█",
        std::string(BYellow) + "█",
        std::string(BBlue) + "█",
        std::string(BPurple) + "█",
        std::string(BCyan) + "█"
    };
    return animatedBar(timePercent, colors, true);
}

std::pair<std::string, std::string> BatteryMonitor::staticAnimation(const int value) const {
    constexpr int width = 20;
    int percent = 0;

    if (value == getCapacity()) {
        percent = getCapacity();
    }
    else if (value == getCycleCount()) {
        percent = 100 - (getCycleCount() / 10);
    }
    else if (value == static_cast<int>(getHealth())) {
        percent = static_cast<int>(getHealth());
    }
    else if (value == getTimeRemaining()) {
        percent = isCharging() ? getTimeRemaining() * 100 / 120 : getTimeRemaining() * 100 / 500;
    }

    std::string color;
    if (percent > 80) color = BGreen;
    else if (percent >= 40) color = BYellow;
    else color = BRed;

    const int filled = percent * width / 100;
    const int empty = width - filled;

    std::vector<std::string> bar(filled, "█");
    bar.insert(bar.end(), empty, "-");

    std::string barStr;
    for (const auto& c : bar) {
        barStr += c;
    }

    return {color, barStr};
}
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

void BatteryMonitor::printStatus(const bool animated) const {
    auto [color, bar] = animatedBattery(getCapacity());

    if (animated) clearScreen();

    std::cout << colorText(BWhite, "\nBattery: ")
                << colorText(C_White, std::to_string(getCapacity()) + "% [" + color + bar + "] ")
                << (isCharging() ? colorText(BGreen, "(charging)") : colorText(BRed, "(discharging)")) << '\n';

    std::cout << colorText(BPurple, "Cycle Count: ")
              << colorText(C_Purple, std::to_string(getCycleCount())) << '\n';

    std::cout << colorText(BCyan, "Health: ")
              << colorText(C_Cyan, std::to_string(getHealth()) + '%') << '\n';

    const int time = getTimeRemaining();
    std::cout << colorText(BBlue, "Time remaining: ");
    std::cout << colorText(C_Blue, (time > 0 ? std::to_string(time) + " minutes" : "calculating...")) << '\n';

    if (animated) {
        std::cout << colorText(BWhite, "\nPress 'q' + Enter to go back.\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
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

std::pair<std::string, std::string> BatteryMonitor::animatedBattery(const int batteryPercent) const {
    constexpr int width = 20;
    static int animationStep = 0;

    std::string color;
    if (batteryPercent > 80) color = BGreen;
    else if (batteryPercent >= 40) color = BYellow;
    else color = BRed;

    const int filled = batteryPercent * width / 100;
    const int empty = width - filled;

    std::vector<std::string> bar(filled, "█");
    bar.insert(bar.end(), empty, "-");

    if (isCharging() && filled > 0) {
        const int animPos = animationStep % filled;
        bar[animPos] = "░";
        ++animationStep;
    }

    std::string barStr;
    for (const auto& c : bar) {
        barStr += c;
    }

    return {color, barStr};
}
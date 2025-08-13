//
// Created by Marat on 13.08.25.
//

#include "BatteryMonitor.h"
#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include <thread>
#include <chrono>

std::string colorText(const std::string& color, const std::string& text, const std::string& reset = C_RESET) {
    return color + text + reset;
}

void BatteryMonitor::execute(const std::vector<std::string>& args) {
    (void)args;

    auto [color, bar] = animatedBattery(getCapacity());
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

    /*
    while (true) {
        auto [color, bar] = animatedBattery(getCapacity());

        std::cout << "\rBattery: " << getCapacity() << "% ["<< color << bar << C_RESET << "] ";
        std::cout << (isCharging() ? " (charging)" : " (discharging)");

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    */
}

int BatteryMonitor::getIntValue(CFStringRef key) {
    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

    int value = -1;
    if (CFArrayGetCount(sources) > 0) {
        CFDictionaryRef desc = IOPSGetPowerSourceDescription(
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
    CFTypeRef blob = IOPSCopyPowerSourcesInfo();
    CFArrayRef sources = IOPSCopyPowerSourcesList(blob);

    bool value = false;
    if (CFArrayGetCount(sources) > 0) {
        CFDictionaryRef desc = IOPSGetPowerSourceDescription(
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

int BatteryMonitor::getRegistryIntValue(CFStringRef key) const {
    io_service_t battery = IOServiceGetMatchingService(
                                kIOMainPortDefault,
                                IOServiceMatching("AppleSmartBattery"));
    if (!battery) return -1;

    CFTypeRef val = IORegistryEntryCreateCFProperty(battery, key, kCFAllocatorDefault, 0);
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
    int designCap = getRegistryIntValue(CFSTR("DesignCapacity"));
    int nominalCap = getRegistryIntValue(CFSTR("NominalChargeCapacity"));
    return (designCap > 0) ? (static_cast<double>(nominalCap) / designCap) * 100 : -1;
}

int BatteryMonitor::getTimeRemaining() const {
    CFStringRef key = isCharging() ? CFSTR(kIOPSTimeToFullChargeKey) : CFSTR(kIOPSTimeToEmptyKey);
    return getIntValue(key);
}

std::pair<std::string, std::string> BatteryMonitor::animatedBattery(int batteryPercent) const {
    constexpr int width = 20;
    static int animationStep = 0;

    std::string color;
    if (batteryPercent > 80) color = C_GREEN;
    else if (batteryPercent >= 40) color = C_YELLOW;
    else color = C_RED;

    int filled = batteryPercent * width / 100;
    int empty = width - filled;

    std::string bar;
    for (int i = 0; i < filled; ++i) {
        bar += "█";
    }
    bar += std::string(empty, '-');

    if (isCharging() && filled > 0) {
        const int animPos = animationStep % filled;
        bar.replace(animPos, 1, "▓");
        ++animationStep;
    }

    return {color, bar};
}
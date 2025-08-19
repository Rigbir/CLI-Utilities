//
// Created by Marat on 19.08.25.
//

#include "SystemInfo.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mach/mach_host.h>
#include <sys/statvfs.h>

void SystemInfo::execute(const std::vector<std::string>& args) {
    (void) args;
    clearScreen();
    for (size_t i = 0; i < 9; ++i) std::cout << '\n';

    const std::vector<std::string> systemInfo = {
        "System Monitoring Utility",
        "",
        "Commands:",
        "  1  - Show CPU, RAM, Disk usages",
        "",
        "Navigation:",
        "  q, quit - go back to main menu"
    };
    printBox(systemInfo);

    std::string input;
    while (true) {
        std::cout << "\n\n" << colorText(BWhite, centered("Enter command: ", termWidth()));
        std::cin >> input;

        if (input == "q" || input == "quit") return;

        try {
            switch (std::stoi(input)) {
                case 1: runLiveMonitoring(); break;
                case 2: getDiskUsage(); break;
                default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
            }
        } catch (const std::invalid_argument&) {
            std::cout << '\n' << colorText(BRed, centered("Please enter a number!\n", termWidth()));
        }
    }

}

void SystemInfo::runLiveMonitoring() {
    std::atomic<bool> stopFlag = false;
    std::thread inputThread([&stopFlag]() {
        std::string line;
        while (!stopFlag) {
            std::getline(std::cin, line);
            if (line == "q" || line == "quit") stopFlag = true;
        }
    });

    while (!stopFlag) {
        getCPUUsage();
    }

    inputThread.join();
}

void SystemInfo::getSystemInfo() {
    //runLiveMonitoring();
    //getCPUUsage();
    // getRAMUsage();
    // getDiskUsage();
    // getTemperature();
}

std::tuple<double, double, double> SystemInfo::CPUUsageCalculation(host_cpu_load_info_data_t& curr) {
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    kern_return_t resultPrev = host_statistics(mach_host_self(),
                                               HOST_CPU_LOAD_INFO,
                                               reinterpret_cast<host_info_t>(&curr),
                                               static_cast<mach_msg_type_number_t*>(&count));

    if (resultPrev == KERN_SUCCESS) {
        const double userTicks =   curr.cpu_ticks[CPU_STATE_USER]   - prev.cpu_ticks[CPU_STATE_USER];
        const double systemTicks = curr.cpu_ticks[CPU_STATE_SYSTEM] - prev.cpu_ticks[CPU_STATE_SYSTEM];
        const double idleTicks =   curr.cpu_ticks[CPU_STATE_IDLE]   - prev.cpu_ticks[CPU_STATE_IDLE];

        const double totalTicks = userTicks + systemTicks + idleTicks;

        const double user = 100 * userTicks / totalTicks;
        const double system = 100 * systemTicks / totalTicks;
        const double idle = 100 * idleTicks / totalTicks;

        prev = curr;
        return {user, system, idle};
    }

    return {0, 0, 0};
}

void SystemInfo::getCPUUsage() {
    clearScreen();

    host_cpu_load_info_data_t curr = {};
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    kern_return_t resultCurr = host_statistics(mach_host_self(),
                                               HOST_CPU_LOAD_INFO,
                                               reinterpret_cast<host_info_t>(&curr),
                                               static_cast<mach_msg_type_number_t*>(&count));

    if (resultCurr == KERN_SUCCESS) {
        auto [user, system, idle] = CPUUsageCalculation(curr);

        auto toString = [](const double val) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << val;
            return ss.str();
        };

        auto print = [&](std::ostream& oss, const std::string& nameProc, const double parameter)->void {
            oss << '\n' << colorText(BWhite, centered(nameProc + toString(parameter) + " %", termWidth()));
        };

        print(std::cout, "User: ", user);
        print(std::cout, "System: ", system);
        print(std::cout, "Idle: ", idle);
        std::cout << "\n\n" << colorText(BWhite, centered("Press 'q' + Enter to go back.", termWidth())) << '\n';

        std::cout << std::flush;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void SystemInfo::getRAMUsage() {

}

void SystemInfo::getDiskUsage() {
    struct statvfs stats;

    auto bytesToGb = [&](const double bytes) {
        return bytes / (1024.0 * 1024.0 * 1024.0);
    };

    if (statvfs("/", &stats) == 0) {
        const double totalBytes = static_cast<double>(stats.f_blocks) * stats.f_frsize;
        const double availableBytes = static_cast<double>(stats.f_bavail) * stats.f_frsize;
        const double usedBytes = totalBytes - availableBytes;

        const double usedPercent = usedBytes > 0 ? (usedBytes / totalBytes) * 100.0 : 0.0;
        const double availablePercent = availableBytes > 0 ? (availableBytes / totalBytes) * 100.0 : 0.0;

        auto toString = [](const double val) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << val;
            return ss.str();
        };

        auto print = [&](std::ostream& oss, const std::string& name, const double value, const double percent) {
            oss << '\n' << colorText(BWhite, centered(name + toString(value) + "Gb" + " (" + toString(percent) + "%)", termWidth()));
        };

        print(std::cout, "      Total: ", bytesToGb(totalBytes), 100.0);
        print(std::cout, "      Used: ", bytesToGb(usedBytes), usedPercent);
        print(std::cout, "Available: ", bytesToGb(availableBytes), availablePercent);
    }
}

void SystemInfo::getTemperature() {

}


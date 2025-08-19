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
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#include <sys/statvfs.h>
#include <sys/mount.h>

double SystemInfo::bytesToMb(const double bytes) {
    return bytes / (1024.0 * 1024.0);
}

double SystemInfo::bytesToGb(const double bytes) {
    return bytes / (1024.0 * 1024.0 * 1024.0);
}

double SystemInfo::bytesToGB(const double bytes) {
    return bytes / 1e9;
}

double SystemInfo::percent(const double val, const double total) {
    return val > 0 ? (val / total) * 100 : 0.0;
}

std::string SystemInfo::toString(const double val) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << val;
    return ss.str();
}

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
                case 3: getRAMUsage(); break;
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
        clearScreen();
        getCPUUsage();
        getRAMUsage();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    inputThread.join();
}

void SystemInfo::getSystemInfo() {
    // runLiveMonitoring();
    // getCPUUsage();
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

        auto print = [&](const std::string& nameProc, const double parameter)->void {
            std::cout << '\n' << colorText(BWhite, centered(nameProc + toString(parameter) + " %", termWidth()));
        };

        print("User: ", user);
        print("System: ", system);
        print("Idle: ", idle);
        //std::cout << "\n\n" << colorText(BWhite, centered("Press 'q' + Enter to go back.", termWidth())) << '\n';

        std::cout << std::flush;
    }

    //std::this_thread::sleep_for(std::chrono::seconds(1));
}

void SystemInfo::getRAMUsage() {
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmStats;
    kern_return_t result = host_statistics64(mach_host_self(),
                                             HOST_VM_INFO64,
                                             reinterpret_cast<host_info64_t>(&vmStats),
                                             static_cast<mach_msg_type_number_t*>(&count));

    if (result != KERN_SUCCESS) {
        std::cerr << "host_statistics failed: " << result << "\n";
        return;
    }

    unsigned long pageSize;
    host_page_size(mach_host_self(), &pageSize);

    const double free =     static_cast<double>(vmStats.free_count)     * pageSize;
    const double active =   static_cast<double>(vmStats.active_count)   * pageSize;
    const double inactive = static_cast<double>(vmStats.inactive_count) * pageSize;
    const double wired =    static_cast<double>(vmStats.wire_count)     * pageSize;
    const double totalUsed = free + active + inactive + wired;

    uint64_t totalPhys = 0;
    size_t len = sizeof(totalPhys);
    sysctlbyname("hw.memsize", &totalPhys, &len, nullptr, 0);
    const double totalInMac = static_cast<double>(totalPhys);

    const std::vector<std::string> outputTable = {
        "RAM Usage:",
        "",
        "Free:          " + toString(bytesToMb(free))       + " MB" + " (" + toString(percent(free, totalInMac))      + "%)",
        "Active:        " + toString(bytesToMb(active))     + " MB" + " (" + toString(percent(active, totalInMac))    + "%)",
        "Inactive:      " + toString(bytesToMb(inactive))   + " MB" + " (" + toString(percent(inactive, totalInMac))  + "%)",
        "Wired:         " + toString(bytesToMb(wired))      + " MB" + " (" + toString(percent(wired, totalInMac))     + "%)",
        "Total:         " + toString(bytesToMb(totalUsed))  + " MB" + " (" + toString(percent(totalUsed, totalInMac)) + "%)",
        "Total In Mac:  " + toString(bytesToMb(totalInMac)) + " MB" + " (100.00%)"
    };

    std::cout << '\n';
    printBox(outputTable);

    std::cout << "\n\n" << colorText(BWhite, centered("Press 'q' + Enter to go back.", termWidth())) << '\n';
    std::cout << std::flush;
    //std::this_thread::sleep_for(std::chrono::seconds(1));
}

void SystemInfo::getDiskUsage() {
    struct statvfs stats;
    struct statfs statsDisk;

    if (statvfs("/", &stats) == 0 && statfs("/", &statsDisk) == 0) {
        const double totalInMac =       static_cast<double>(statsDisk.f_blocks) * statsDisk.f_bsize;
        const double totalBytes =       static_cast<double>(stats.f_blocks) * stats.f_frsize;
        const double availableBytes =   static_cast<double>(stats.f_bavail) * stats.f_frsize;
        const double usedBytes =        totalBytes - availableBytes;

        const double totalPercent = totalBytes > 0 ? (bytesToGb(totalBytes) / bytesToGB(totalInMac)) * 100.0 : 0.0;

        const std::vector<std::string> outputTable = {
            "Disk Usage:",
            "",
            "User:          " + toString(bytesToGb(usedBytes))      + " GB" + " (" + toString(percent(usedBytes, totalBytes))      + "%)",
            "Available:     " + toString(bytesToGb(availableBytes)) + " GB" + " (" + toString(percent(availableBytes, totalBytes)) + "%)",
            "Total:         " + toString(bytesToGb(totalBytes))     + " GB" + " (" + toString(totalPercent)                                + "%)",
            "Total In Mac:  " + toString(bytesToGB(totalInMac))     + " GB" + " (" + toString(100.0)                                   + "%)"
        };

        std::cout << '\n';
        printBox(outputTable);
    }
}

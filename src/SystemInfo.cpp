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
#include <csignal>

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

std::string SystemInfo::getMacVersion(const char* name) {
    char buf[256];
    size_t size = sizeof(buf);
    if (sysctlbyname(name, buf, &size, nullptr, 0) == 0) {
        return std::string(buf, size - 1);
    }
    return {};
}

std::string SystemInfo::getMacName() {
    FILE* pipe = popen("sw_vers -productName", "r");
    if (!pipe) return {};
    char buf[128];
    std::string result;
    while (fgets(buf, sizeof(buf), pipe)) {
        result += buf;
    }
    pclose(pipe);
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
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
        "  2  - Process monitoring (top by CPU/RAM)",
        "",
        "Navigation:",
        "  q, quit - go back to main menu"
    };
    printBox(systemInfo);

    productName = getMacName();
    productVersion = getMacVersion("kern.osproductversion");

    std::string input;
    while (true) {
        std::cout << "\n\n" << colorText(BWhite, centered("Enter command: ", termWidth()));
        std::cin >> input;

        if (input == "q" || input == "quit") return;

        try {
            switch (std::stoi(input)) {
                case 1: runLiveMonitoring(); break;
                case 2: runProcessMonitoring(); break;
                default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth()));
            }
        } catch (const std::invalid_argument&) {
            std::cout << '\n' << colorText(BRed, centered("Please enter a number!\n", termWidth()));
        }
    }
}

void SystemInfo::runLiveMonitoring() {
    std::atomic<bool> stopFlag = false;

    while (!stopFlag) {
        clearScreen();
        for (size_t i = 0; i < 9; ++i) std::cout << '\n';
        std::cout << colorText(BWhite, centered("Product Name: "     + productName +
                                                       "   Product Version: " + productVersion, termWidth())) << "\n\n";

        auto cpuTable = getCPUUsage();
        auto ramTable = getRAMUsage();
        auto diskTable = getDiskUsage();
        printBoxes({cpuTable, ramTable, diskTable});

        std::cout << "\n\n" << colorText(BWhite, centered("Press 'q' to go back: ", termWidth()));
        std::cout << std::flush;

        char c = getCharNonBlocking();
        if (c == 'q') stopFlag = true;

        for (int i = 0; i < 10 && !stopFlag; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void SystemInfo::runProcessMonitoring() {
    std::cout << '\n' << colorText(BWhite, centered("Enter count of process: ", termWidth()));
    std::cin >> limits;
    if (limits <= 0) {
        std::cout << '\n' << colorText(BRed, centered("Invalid process count!", termWidth()));
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::atomic<bool> stopFlag = false;
    std::atomic<bool> waitingForInput = false;

    std::thread inputThread([&stopFlag, this, &waitingForInput]() {
        std::string line;
        while (!stopFlag) {
            if (!waitingForInput) {
                char c = getCharNonBlocking();
                if (c != 0) {
                    if (c == 'q') {
                        stopFlag = true;
                    } else if (c == 'y' || c == 'Y') {
                        waitingForInput = true;

                        std::cout << "\n\n" << colorText(BWhite, centered("Enter number process: ", termWidth()));
                        std::getline(std::cin, line);

                        try {
                            const int numberProcess = std::stoi(line);
                            if (numberProcess >= 0 && numberProcess <= static_cast<int>(this->idProcess.size())) {
                                if (kill(this->idProcess[numberProcess], SIGTERM) == 0) {
                                    std::cout << '\n' << colorText(BGreen, centered("Process killed", termWidth()));
                                } else {
                                    std::cout << '\n' << colorText(BRed, centered("Killed failed", termWidth()));
                                }
                            } else {
                                std::cout << '\n' << colorText(BRed, centered("Invalid input process number!", termWidth()));
                            }
                        } catch (...) {
                            std::cout << '\n' << colorText(BRed, centered("Wrong input!", termWidth()));
                        }

                        waitingForInput = false;
                    } else {
                        line += c;
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            }
        }
    });

    while (!stopFlag) {
        if (!waitingForInput) {
            topByCpuRam();

            std::cout << "\n\n" << colorText(BWhite, centered("You want to kill the process [y/q (to quit)]: ", termWidth()));
            std::cout << std::flush;
        }

        for (int i = 0; i < 30 && !stopFlag; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    inputThread.join();
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

std::vector<std::string> SystemInfo::getCPUUsage() {
    host_cpu_load_info_data_t curr = {};
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    kern_return_t resultCurr = host_statistics(mach_host_self(),
                                               HOST_CPU_LOAD_INFO,
                                               reinterpret_cast<host_info_t>(&curr),
                                               static_cast<mach_msg_type_number_t*>(&count));

    if (resultCurr != KERN_SUCCESS) {
        std::cerr << "host_statistics failed: " << resultCurr << '\n';
    }

    auto [user, system, idle] = CPUUsageCalculation(curr);

    return {
        "CPU Usage:",
        "",
        "User:    " + toString(user)   + " %",
        "System:  " + toString(system) + " %",
        "Idle:    " + toString(idle)   + " %"
    };
}

std::vector<std::string> SystemInfo::getRAMUsage() {
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmStats;
    kern_return_t result = host_statistics64(mach_host_self(),
                                             HOST_VM_INFO64,
                                             reinterpret_cast<host_info64_t>(&vmStats),
                                             static_cast<mach_msg_type_number_t*>(&count));

    if (result != KERN_SUCCESS) {
        std::cerr << "host_statistics failed: " << result << '\n';
    }

    unsigned long pageSize;
    host_page_size(mach_host_self(), &pageSize);

    const double free =     static_cast<double>(vmStats.free_count)     * static_cast<double>(pageSize);
    const double active =   static_cast<double>(vmStats.active_count)   * static_cast<double>(pageSize);;
    const double inactive = static_cast<double>(vmStats.inactive_count) * static_cast<double>(pageSize);;
    const double wired =    static_cast<double>(vmStats.wire_count)     * static_cast<double>(pageSize);;
    const double totalUsed = free + active + inactive + wired;

    uint64_t totalPhys = 0;
    size_t len = sizeof(totalPhys);
    sysctlbyname("hw.memsize", &totalPhys, &len, nullptr, 0);
    const auto totalInMac = static_cast<double>(totalPhys);

    return {
        "RAM Usage:",
        "",
        "Free:          " + toString(bytesToMb(free))       + " MB" + " (" + toString(percent(free, totalInMac))      + "%)",
        "Active:        " + toString(bytesToMb(active))     + " MB" + " (" + toString(percent(active, totalInMac))    + "%)",
        "Inactive:      " + toString(bytesToMb(inactive))   + " MB" + " (" + toString(percent(inactive, totalInMac))  + "%)",
        "Wired:         " + toString(bytesToMb(wired))      + " MB" + " (" + toString(percent(wired, totalInMac))     + "%)",
        "",
        "Total:         " + toString(bytesToMb(totalUsed))  + " MB" + " (" + toString(percent(totalUsed, totalInMac)) + "%)",
        "Total In Mac:  " + toString(bytesToMb(totalInMac)) + " MB" + " (100.00%)"
    };
}

std::vector<std::string> SystemInfo::getDiskUsage() {
    struct statfs statsDisk;
    struct statvfs stats;

    if (statvfs("/", &stats) == 0 && statfs("/", &statsDisk) == 0) {
        const double totalInMac =       static_cast<double>(statsDisk.f_blocks) * statsDisk.f_bsize;
        const double totalBytes =       static_cast<double>(stats.f_blocks)     * static_cast<double>(stats.f_frsize);
        const double availableBytes =   static_cast<double>(stats.f_bavail)     * static_cast<double>(stats.f_frsize);
        const double usedBytes =        totalBytes - availableBytes;

        const double totalPercent = totalBytes > 0 ? (bytesToGb(totalBytes) / bytesToGB(totalInMac)) * 100.0 : 0.0;

        return {
            "Disk Usage:",
            "",
            "User:          " + toString(bytesToGb(usedBytes))      + " GB" + " (" + toString(percent(usedBytes, totalBytes))      + "%)",
            "Available:     " + toString(bytesToGb(availableBytes)) + " GB" + " (" + toString(percent(availableBytes, totalBytes)) + "%)",
            "",
            "",
            "",
            "Total:         " + toString(bytesToGb(totalBytes))     + " GB" + " (" + toString(totalPercent)                                + "%)",
            "Total In Mac:  " + toString(bytesToGB(totalInMac))     + " GB" + " (" + toString(100.0)                                   + "%)"
        };
    }
    return {};
}

void SystemInfo::topByCpuRam() {
    const std::string command = R"(ps axo pid,pcpu,pmem,comm | grep -vE 'ps|sort|head|Helper|Renderer|Backend|gitstatusd' | sort -k 2 -nr | head -n)" + std::to_string(limits);

    const std::string output = getCommand(command);
    if (output.empty()) {
        std::cout << '\n' << colorText(BRed, "Error executing command or empty result.");
        return;
    }

    std::istringstream outputStream(output);
    std::string line;

    std::vector<std::vector<std::string>> outputTable;
    outputTable.push_back({"№", "CPU %", "RAM %", "Command"});

    int index = 1;
    while (std::getline(outputStream, line)) {
        std::istringstream iss(line);
        std::string idStr, cpuStr, ramStr, comName;

        if (iss >> idStr >> cpuStr >> ramStr) {
            if (idStr.empty() || cpuStr.empty() || ramStr.empty()) continue;

            std::getline(iss, comName);
            if (!comName.empty() && comName[0] == ' ') comName.erase(0, 1);

            this->idProcess[index] = std::stoi(idStr);

            outputTable.push_back({
                std::to_string(index),
                (std::ostringstream() << std::fixed << std::setprecision(1) << std::stod(cpuStr)).str(),
                (std::ostringstream() << std::fixed << std::setprecision(1) << std::stod(ramStr)).str(),
                shortPath(comName)
            });
            ++index;
        }
    }

    clearScreen();
    for (size_t i = 0; i < 11; ++i) std::cout << '\n';
    std::cout << colorText(BWhite, centered("Product Name: "     + productName +
                                                    "   Product Version: "  + productVersion, termWidth())) << "\n\n";
    printProcessTable(outputTable);
}

std::string SystemInfo::getCommand(const std::string& command) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << '\n' << colorText(BRed, "Unable to execute command.");
        return "";
    }

    char buf[256];
    std::ostringstream result;

    while (fgets(buf, sizeof(buf), pipe) != nullptr) {
        result << buf;
    }
    pclose(pipe);

    return result.str();
}
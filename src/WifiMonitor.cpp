//
// Created by Marat on 13.08.25.
//

#include "WifiMonitor.h"
#include <iostream>
#include <memory>
#include <array>

void printOutput(const std::string& result) {
    std::cout << "\033[2J\033[H";
    std::cout << '\n' << colorText(BWhite, result + '\n');
}

std::string WifiMonitor::trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }

    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }

    return s.substr(start, end - start);
}

std::string WifiMonitor::runCommandWdutil(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;

    const std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return colorText(BRed, "Error: cannot open pipe");
    }

    bool skip = false;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line = buffer.data();
        if (line.find("NETWORK") != std::string::npos) {
            skip = true;
            continue;
        }

        if (skip && line.find("WIFI") != std::string::npos) {
            skip = false;
        }

        if (line.find("AWDL") != std::string::npos) {
            break;
        }

        if (!skip) {
            result += line;
        }
    }

    return result;
}

std::string WifiMonitor::runCommandSystem(
                  const std::string& cmd
                , const std::string& startMarker
                , const std::string& endMarker) {

    std::array<char, 128> buffer;
    std::string result;

    const std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return colorText(BRed, "Error: cannot open pipe");
    }

    bool skip = false;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line = buffer.data();

        if (!skip && line.find(startMarker) != std::string::npos) {
            skip = true;
            continue;
        }

        if (skip && line.find(endMarker) != std::string::npos) {
            break;
        }

        if (skip) {
            result += trim(line) + '\n';
        }
    }

    return result;
}

void WifiMonitor::execute(const std::vector<std::string>& args) {
    (void) args;

    while (true) {
        std::cout << colorText(BWhite, "\nChoose one [1, 2, 3]: ");
        int inputChoose;
        std::cin >> inputChoose;

        switch (inputChoose) {
            case 1: showCurrentConnections(); return;
            case 2: listConnections(); return;
            case 3: monitorConnections(); return;
            default: std::cout << colorText(BRed, "\nWrong input!\n"); break;
        }
    }
}

void WifiMonitor::showCurrentConnections() const {
    const std::string output = runCommandWdutil("sudo wdutil info");
    std::cout << '\n' << colorText(BWhite, output + '\n');
}

void WifiMonitor::listConnections() const {
    while (true) {
        printOutput(runCommandSystem("system_profiler SPAirPortDataType",
                                                    "Other Local Wi-Fi Networks:", "awdl0:"));
    }
}

void WifiMonitor::monitorConnections() const {
    while (true) {
        printOutput(runCommandSystem("system_profiler SPAirPortDataType",
                                                    "Current Network Information:", "Other Local Wi-Fi Networks:"));
    }
}

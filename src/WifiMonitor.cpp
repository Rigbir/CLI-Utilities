//
// Created by Marat on 13.08.25.
//

#include "WifiMonitor.h"
#include <iostream>
#include <memory>
#include <array>
#include <thread>
#include <chrono>
#include <atomic>

void WifiMonitor::printOutput(const std::string& result) {
    clearScreen();
    for (size_t i = 0; i < 9; ++i) std::cout << '\n';

    std::string line;
    std::stringstream ss(result);
    std::vector<std::string> outputTable;

    while (std::getline(ss, line)) {
        line = trim(line);
        outputTable.push_back(line);
    }

    printBox(outputTable);
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

std::string WifiMonitor::runCommandSystem(const std::string& cmd,
                                          const std::string& startMarker,
                                          const std::string& endMarker) {

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

void WifiMonitor::runLiveMonitor(const std::string& command,
                                 const std::string& startMarker,
                                 const std::string& endMarker,
                                 const std::string& message = "Press 'q' to go back: ") {

    std::atomic<bool> stopFlag = false;

    while (!stopFlag) {
        printOutput(runCommandSystem(command, startMarker, endMarker));
        std::cout << '\n' << colorText(BWhite, centered(message, termWidth()));

        char c = getCharNonBlocking();
        if (c == 'q') stopFlag = true;

        std::cout << std::flush;
    }
}

void WifiMonitor::execute([[maybe_unused]] const std::vector<std::string>& args) {
    clearScreen();
    for (size_t i = 0; i < 9; ++i) std::cout << '\n';

    const std::vector<std::string> wifiInfo = {
        "Wi-Fi Monitoring Utility",
        "",
        "Commands:",
        "  1  - Show current connection",
        "  2  - List available networks",
        "  3  - Monitor connections",
        "",
        "Navigation:",
        "  q, quit - go back to main menu"
    };
    printBox(wifiInfo);

    std::string input;
    while (true) {
        std::cout << "\n\n" << colorText(BWhite, centered("Enter command: ", termWidth()));
        std::cin >> input;

        if (input == "q" || input == "quit") return;

        try {
            switch (std::stoi(input)) {
                case 1: showCurrentConnections(); break;
                case 2: listConnections(); break;
                case 3: monitorConnections(); break;
                default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
            }
        } catch (const std::invalid_argument&) {
            std::cout << '\n' << colorText(BRed, centered("Please enter a number!\n", termWidth()));
        }
    }
}

void WifiMonitor::showCurrentConnections() const {
    const std::string output = runCommandWdutil("sudo wdutil info");
    std::cout << '\n' << colorText(BWhite, output + '\n');
}

void WifiMonitor::listConnections() const {
    runLiveMonitor("system_profiler SPAirPortDataType",
                   "Other Local Wi-Fi Networks:", "awdl0:");
}

void WifiMonitor::monitorConnections() const {
    runLiveMonitor("system_profiler SPAirPortDataType",
                                 "Current Network Information:", "Other Local Wi-Fi Networks:");
}

//
// Created by Marat on 19.08.25.
//

#pragma once
#include "SystemInfo.h"
#include <iostream>
#include <mach/mach_host.h>
#include <mach/host_info.h>

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
                case 1: getSystemInfo(); break;
                default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
            }
        } catch (const std::invalid_argument&) {
            std::cout << '\n' << colorText(BRed, centered("Please enter a number!\n", termWidth()));
        }
    }

}

void SystemInfo::getSystemInfo() const {
    getCPUUsage();
    getRAMUsage();
    getDiskUsage();
    getTemperature();
}

void SystemInfo::getCPUUsage() const {

}

void SystemInfo::getRAMUsage() const {

}

void SystemInfo::getDiskUsage() const {

}

void SystemInfo::getTemperature() const {

}


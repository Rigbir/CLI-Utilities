//
// Created by Marat on 17.08.25.
//

#include "CodeCounter.h"
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

size_t CodeCounter::countLine(const std::string& folderPath) {
    std::ifstream file(folderPath);
    if (!file.is_open()) return 0;

    size_t count = 0;
    std::string line;
    while (std::getline(file, line)) {
        ++count;
    }

    return count;
}

bool CodeCounter::isIgnorePath(const fs::path& path) {
    for (auto p : path) {
        std::string folder = toLower(p.filename().string());
        if (folder == "cmake-build-debug" || folder == ".git" || folder == "build" ||
            folder == "cmakefiles" || folder == ".vscode" || folder == ".idea" ||
            folder.find("cmake") != std::string::npos) {
            return true;
        }
    }
    return false;
}

void CodeCounter::execute(const std::vector<std::string>& args) {
    (void) args;
    clearScreen();
    for (size_t i = 0; i < 9; ++i) std::cout << '\n';

    const std::vector<std::string> batteryInfo = {
        "Code Counter Utility",
        "",
        "Commands:",
        "  1  - Show project line count",
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
            case 1: getFolderStats(); break;
            default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
        }
    }
}

void CodeCounter::getFolderStats() const {
    std::string folderPath;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << '\n' << colorText(BWhite, centered("Write path to project Directory: ", termWidth()));
    std::getline(std::cin, folderPath);

    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cout << '\n' << colorText(BRed, centered("Invalid Path.", termWidth()));
        return;
    }

    std::vector<FileStats> headers;
    std::vector<FileStats> sources;

    size_t totalHeadersLine = 0;
    size_t totalSourcesLine = 0;

    for (auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (!entry.is_regular_file() || isIgnorePath(entry.path().parent_path())) continue;

        auto path = entry.path();
        auto extension = path.extension().string();

        if (extension == ".h" || extension == ".hpp" || extension == ".tpp") {
            const size_t lines = countLine(path);
            headers.push_back({path.filename().string(), lines});
            totalHeadersLine += lines;
        }

        if (extension == ".cpp" || extension == ".cc" || extension == ".cxx") {
            const size_t lines = countLine(path);
            sources.push_back({path.filename().string(), lines});
            totalSourcesLine += lines;
        }
    }

    std::cout << '\n';
    printTable(headers, sources, totalHeadersLine, totalSourcesLine);
}

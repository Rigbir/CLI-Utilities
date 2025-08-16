//
// Created by Marat on 13.08.25.
//

#include "Cleaner.h"
#include <iostream>
#include <map>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

std::string Cleaner::getFolder() {
    std::string inputFolder;
    std::cout << '\n' << colorText(BWhite, centered("Write Folder [cache, xCode, safari]: ", termWidth()));
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, inputFolder);
    return toLower(inputFolder);
}

std::string Cleaner::resolveFolderPath(const std::string& key) {
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        return colorText(BRed, "Couldn't identify the home directory.\n");
    }

    if (key == "cache") return static_cast<std::string>(homeDir) + "/Library/Caches/";
    if (key == "xcode") return static_cast<std::string>(homeDir) + "/Library/Developer/Xcode/DerivedData/";
    if (key == "safari") return static_cast<std::string>(homeDir) + "/Library/Safari/LocalStorage/";
    return "";
}

void Cleaner::printFileInFolder(const std::string& folder) {
    DIR* dir = opendir(folder.c_str());
    std::cout << colorText(BRed, "\nDirectory: " + folder) << "\n\n" << std::string(70, '-') << '\n';

    if (!dir) {
        std::cerr << colorText(BRed, "Cannot open directory: " + folder) << '\n';
        return;
    }

    struct dirent* entry;
    std::vector<std::string> names;
    while ((entry = readdir(dir)) != nullptr) {
        if (std::string name = entry->d_name; name != "." && name != "..") {
            names.push_back(name);
            removeEntries[folder][name] = getStats(folder + name);
        }
    }
    closedir(dir);

    if (names.empty()) {
        std::cout << colorText(BYellow, "\nDirectory is empty.\n");
    }

    std::sort(names.begin(), names.end());

    for (const auto& name : names) {
        std::string stats = getStats(folder + name);
        std::cout << colorText(BCyan, name)
                  << std::string(70 - name.size(), ' ')
                  << stats << '\n';
    }
}

std::string formatTime(time_t t) {
    char buf[64];
    strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", localtime(&t));
    return std::string(buf);
}

bool confirmation() {
    char confirm;
    while (true) {
        std::cout << '\n' << colorText(BRed, centered("Are you sure? [y/n]: ", termWidth()));
        std::cin >> confirm;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (confirm == 'y' || confirm == 'Y') return true;
        if (confirm == 'n' || confirm == 'N') return false;

        std::cout << colorText(BYellow, "Invalid input. Please enter 'y' or 'n'.\n");
    }
}

void Cleaner::execute(const std::vector<std::string>& args) {
    (void) args;
    clearScreen();
    for (size_t i = 0; i < 9; ++i) std::cout << '\n';

    const std::vector<std::string> cleanerInfo = {
        "Cleaner Utility",
        "",
        "Commands:",
        "  1  - Show all info",
        "  2  - Remove files from directory",
        "",
        "Navigation:",
        "  q, quit - go back to main menu"
    };
    printBox(cleanerInfo);

    std::string input;
    while (true) {
        std::cout << "\n\n" << colorText(BWhite, centered("Enter command: ", termWidth()));
        std::cin >> input;

        if (input == "q" || input == "quit") return;

        switch (std::stoi(input)) {
            case 1: getAllInfo(); break;
            case 2: removeFile(); break;
            default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
        }
    }
}

void Cleaner::getAllInfo() {
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        std::cerr << colorText(BRed, "Couldn't identify the home directory.") << '\n';
        return;
    }

    const std::vector<std::string> folders = {
        static_cast<std::string>(homeDir) + "/Library/Caches/",
        static_cast<std::string>(homeDir) + "/Library/Developer/Xcode/DerivedData/",
        static_cast<std::string>(homeDir) + "/Library/Safari/LocalStorage/"
    };

    std::map<std::string, std::string> entries;
    for (const auto& folder : folders) {
        entries.clear();
        DIR* dir = opendir(folder.c_str());
        std::cout << colorText(BRed, "\nDirectory: " + folder) << "\n\n" << std::string(70, '-') << '\n';

        if (!dir) {
            std::cout << colorText(BRed, "\nNo such file or directory") << '\n';
            continue;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (std::string name = entry->d_name; name != "." && name != "..") {
                entries[name] = getStats(folder + name);
                allEntries[folder][name] = getStats(folder + name);
            }
        }
        closedir(dir);

        if (entries.empty()) {
            std::cout << colorText(BYellow, "\nDirectory is empty.\n\n");
        }

        for (const auto& [filename, stats] : entries) {
            std::cout << std::left
                      << colorText(BCyan, filename)
                      << std::string(70 - filename.size(), ' ')
                      << stats << '\n';
        }
        std::cout << std::string(70, '-') << '\n';
    }
}

std::string Cleaner::getStats(const std::string& path) const {
    struct stat st{};
    if (stat(path.c_str(), &st) != 0) {
        return colorText(BRed, "Cannot stat file: " + path);
    }

    std::ostringstream oss;

    switch (st.st_mode & S_IFMT) {
        case S_IFDIR: oss << colorText(BBlue, "[DIR] "); break;
        case S_IFREG: oss << colorText(BGreen, "[FILE] "); break;
        default:      oss << colorText(BYellow, "[OTHER] "); break;
    }

    oss << colorText(BWhite, "Size: ") << colorText(BBlue, std::to_string(st.st_size) + " bytes  ")
        << colorText(BWhite,"Modified: ") << colorText(BBlue, formatTime(st.st_mtime)) + "  ";

    return oss.str();
}

void Cleaner::removeFile() {
    const std::string folder = getFolder();
    const std::string workFolder = resolveFolderPath(folder);

    if (workFolder.empty()) {
        std::cerr << '\n' << colorText(BRed, centered(("Unknown folder key: " + folder), termWidth())) << '\n';
        return;
    }

    printFileInFolder(workFolder);

    while (true) {
        std::cout << '\n' << colorText(BWhite, centered("Enter filename to remove or 'quit' for Exit: ", termWidth()));
        std::string input;
        std::getline(std::cin, input);
        std::string inputLower = toLower(input);

        if (inputLower == "quit") return;

        bool exactFound = false;
        std::vector<std::string> matches;

        for (const auto& [filename, _] : removeEntries[workFolder]) {
            std::string filenameLower = filename;
            filenameLower = toLower(filenameLower);
            
            if (filenameLower == inputLower) {
                matches = { filename };
                exactFound = true;
                break;
            }
        }

        if (!exactFound) {
            for (const auto& [filename, _] : removeEntries[workFolder]) {
                std::string filenameLower = filename;
                filenameLower = toLower(filenameLower);

                if (filenameLower.find(inputLower) != std::string::npos) {
                    matches.push_back(filename);
                }
            }
        }

        if (matches.empty()) {
            std::cout << '\n' << colorText(BRed, centered("No match found!\n", termWidth()));
            continue;
        } else if (matches.size() == 1) {
            std::cout << '\n' << colorText(BWhite, centered(("Find file: " + matches[0]), termWidth())) << '\n';
            if (confirmation()) {
                std::filesystem::is_directory(workFolder)
                ? std::filesystem::remove_all((workFolder + matches[0]).c_str())
                : std::filesystem::remove((workFolder + matches[0]).c_str());

                removeEntries.erase(matches[0]);
                std::cout << '\n' << colorText(BYellow, centered("File was deleted.\n", termWidth()));
            }
            break;
        } else {
            std::cout << '\n' << colorText(BWhite, centered("Multiple matches found:\n", termWidth()));
            for (const auto& file : matches) {
                std::cout << colorText(BCyan, file) << '\n';
            }
            std::cout << '\n' << colorText(BWhite, centered("Please enter more specific name.\n", termWidth()));        }
    }
}
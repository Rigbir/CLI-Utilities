//
// Created by Marat on 13.08.25.
//

#include "Cleaner.h"
#include <iostream>
#include <map>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <numeric>

std::string Cleaner::getFolder() {
    std::string inputFolder;
    std::cout << '\n' << colorText(BWhite, centered("Write Folder [cache, xCode, safari]: ", termWidth()));
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
    std::cout << colorText(BRed, "\nDirectory: " + folder) << "\n\n" << std::string(termWidth(), '-') << '\n';

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

std::string formatTime(const time_t t) {
    char buf[64];
    strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", localtime(&t));
    return std::string(buf);
}

bool Cleaner::confirmation(const std::string& text) {
    char confirm;
    while (true) {
        std::cout << '\n' << colorText(BRed, centered(text, termWidth()));
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
        "  1  - Cache Info & Management (view and delete cache files)",
        "  2  - Large Directory Scan (view size, optionally remove files)",
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

        try {
            switch (std::stoi(input)) {
                case 1: getAllInfo(); break;
                case 2: largeDirectory(); break;
                default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
            }
        } catch (const std::invalid_argument&) {
            std::cout << '\n' << colorText(BRed, centered("Please enter a number!\n", termWidth()));
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
        std::cout << colorText(BRed, "\nDirectory: " + folder) << "\n\n" << std::string(termWidth(), '-') << '\n';

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
        std::cout << std::string(termWidth(), '-') << '\n';
    }

    removeFile();
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
    if (!confirmation("Do you want to delete a directory? [y/n]: ")) return;

    std::string folder = getFolder();
    std::string workFolder = resolveFolderPath(folder);

    while (workFolder.empty()) {
        std::cerr << '\n' << colorText(BRed, centered(("Unknown folder key: " + folder), termWidth())) << '\n';
        folder = getFolder();
        workFolder = resolveFolderPath(folder);
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
            if (confirmation("Are you sure? [y/n]: ")) {
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

double bytesToGb(const long long bytes) {
    return bytes / (1024.0 * 1024.0 * 1024.0);
}

std::string roundGb(const double size) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size;
    return ss.str();
}

void Cleaner::largeDirectory() {
    std::cout << '\n' << colorText(BYellow, centered("Please wait, calculating sizes...", termWidth())) << std::endl;

    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        std::cerr << colorText(BRed, "Couldn't identify the home directory.") << '\n';
        return;
    }

    std::map<fs::path, long long> dirSizes;
    for (const auto& entry : fs::recursive_directory_iterator(homeDir,
                                                               fs::directory_options::skip_permission_denied)) {
        try {
            if (entry.is_regular_file()) {
                dirSizes[entry.path()] = entry.file_size();
            }
        } catch (...) {}
    }

    std::map<fs::path, long long> totalSizes;
    for (const auto& [path, size] : dirSizes) {
        fs::path p = path.parent_path();
        while (!p.empty() && p.string().find(homeDir) == 0) {
            totalSizes[p] += size;
            p = p.parent_path();
        }
    }

    std::vector<std::vector<std::string>> outputDate;
    outputDate.push_back({"№", "Directory", "Size (Gb)"});

    for (const auto& [dir, size] : totalSizes) {
        double sizeGb = bytesToGb(size);
        if (sizeGb >= 5.0) {
            outputDate.push_back({
                std::to_string(0),
                shortPath(dir.string()),
                roundGb(sizeGb)
            });
        }
    }

    std::sort(outputDate.begin() + 1, outputDate.end(), [](const std::vector<std::string>& a,
                                                                          const std::vector<std::string>& b) {
        const double sizeA = std::stod(a[2]);
        const double sizeB = std::stod(b[2]);
        return sizeA > sizeB;
    });

    for (size_t i = 1; i < outputDate.size(); ++i) {
        outputDate[i][0] = std::to_string(i);
    }

    std::cout << '\n';
    printProcessTable(outputDate);
}

std::string Cleaner::shortPath(const std::string& path) {
    if (path.empty()) return "";

    std::vector<std::string> segments;
    std::stringstream ss(path);
    std::string item;
    while (std::getline(ss, item, '/')) {
        if (!item.empty()) segments.push_back(item);
    }

    if (segments.size() <= 4) {
        return std::accumulate(segments.begin(), segments.end(), std::string(),
                               [](const std::string& a, const std::string& b){ return a + "/" + b; });
    }

    std::string startPart = "/" + segments[0] + "/" + segments[1] + "/" + segments[2];
    std::string endPart = segments[segments.size()-2] + "/" + segments[segments.size()-1];

    return startPart + "/.../" + endPart;
}

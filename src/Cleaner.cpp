//
// Created by Marat on 13.08.25.
//

#include "Cleaner.h"
#include <iostream>
#include <map>
#include <iomanip>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

void Cleaner::execute(const std::vector<std::string>& args) {
    (void) args;
    getAllInfo();

    char del;
    std::cout << colorText(BWhite, "Are you want delete dir/path? [y/n]: ");
    std::cin >> del;
    if (del == 'y' || del == 'Y') {
        std::string folder;
        std::cout << colorText(BWhite, "\nWrite Folder: [cache, xCode, safari] ");
        std::cin >> folder;
        removeFile(folder);
    } else if (del == 'n' || del == 'N') {
        return;
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

    for (const auto& folder : folders) {
        DIR* dir = opendir(folder.c_str());

        std::cout << colorText(BRed, "\nDirectory: " + folder) << "\n\n" << std::string(70, '-') << '\n';

        if (!dir) {
            std::cout << colorText(BRed, "\nNo such file or directory") << '\n';
            continue;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (std::string name = entry->d_name; name != "." && name != "..") {
                this->entries[name] = getStats(folder + name);
            }
        }
        closedir(dir);

        for (const auto& [filename, stats] : entries) {
            std::cout << std::left
                      << colorText(BCyan, filename)
                      << std::string(70 - filename.size(), ' ')
                      << stats << '\n';
        }
        std::cout << std::string(70, '-') << '\n';
    }
}

std::string formatTime(time_t t) {
    char buf[64];
    strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", localtime(&t));
    return std::string(buf);
}

std::string Cleaner::getStats(const std::string& path) const {
    struct stat st{};
    if (stat(path.c_str(), &st) != 0) {
        return colorText(BRed, "Cannot stat file: " + path);
    }

    std::string result;
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        result += colorText(BBlue, "[DIR] ") + colorText(BWhite, " ");
    } else if ((st.st_mode & S_IFMT) == S_IFREG) {
        result += colorText(BGreen, "[FILE] ") + colorText(BWhite, " ");
    } else {
        result += colorText(BYellow, "[OTHER] ") + colorText(BWhite, " ");
    }

    result += colorText(BWhite, "Size: ") + colorText(BBlue, std::to_string(st.st_size) + " bytes ") + "  ";
    result += colorText(BWhite," Modified: ") + colorText(BBlue, formatTime(st.st_mtime)) + "  ";

    return result;
}

bool confirmation() {
    char confirm;
    std::cout << colorText(BRed, "\nAre you sure? [y/n]: ");
    std::cin.ignore();
    std::cin >> confirm;
    bool answer = false;
    if (confirm == 'y' || confirm == 'Y') {
        answer = true;
    }
    return answer;
}

void Cleaner::removeFile(const std::string& folder) {
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        std::cerr << colorText(BRed, "Couldn't identify the home directory.") << '\n';
        return;
    }

    std::string workFolder;
    if (folder == "cache") {
        workFolder = static_cast<std::string>(homeDir) + "/Library/Caches/";
    } else if (folder == "xCode") {
        workFolder = static_cast<std::string>(homeDir) + "/Library/Developer/Xcode/DerivedData/";
    } else if (folder == "safari") {
        workFolder = static_cast<std::string>(homeDir) + "/Library/Safari/LocalStorage/";
    }

    std::string input;
    std::cout << colorText(BWhite, "Enter filename to remove: ");
    std::cin.ignore();
    std::getline(std::cin, input);

    std::vector<std::string> matches;
    for (const auto& [filename, _] : entries) {
        if (filename.find(input) == 0) {
            matches.push_back(filename);
        }
    }

    if (matches.empty()) {
        std::cout << colorText(BRed, "No match found!\n");
    } else if (matches.size() == 1) {
        std::cout << colorText(BWhite, "Find file: " + matches[0]) << '\n';
        if (confirmation()) {
            std::remove((workFolder + matches[0]).c_str());
        } else {
            return;
        }
        entries.erase(matches[0]);
    } else {
        std::cout << colorText(BWhite, "Multiple matches found:\n");
        for (const auto& file : matches) {
            std::cout << colorText(BCyan, file) << '\n';
        }
        removeFile(folder);
    }
}
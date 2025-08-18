//
// Created by Marat on 17.08.25.
//

#include "CodeCounter.h"
#include <unordered_set>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

void CodeCounter::initMap() {
    languageMap = {
        {"cpp", { {".h", ".hpp", ".tpp", ".cpp", ".cc", ".cxx"}, "C++"}},
        {"java", { {".java"}, "Java"}},
        {"python", { {".py"}, "Python"}},
        {"js", { {".js", ".jsx"}, "JavaScript"}},
        {"ts", { {".ts", ".tsx"}, "TypeScript"}},
        {"cs", { {".cs"}, "C#"}},
        {"go", { {".go"}, "Go"}},
        {"rust", { {".rs"}, "Rust"}},
        {"swift", { {".swift"}, "Swift"}},
        {"kotlin", { {".kt", ".kts"}, "Kotlin"}},
        {"php", { {".php"}, "PHP"}},
        {"ruby", { {".rb"}, "Ruby"}},
        {"assembly", { {".asm"}, "Assembly"}}
    };
}

std::string CodeCounter::inputFolder() {
    std::string folderPath;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << '\n' << colorText(BWhite, centered("Write path to project Directory: ", termWidth()));
    std::getline(std::cin, folderPath);

    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cout << '\n' << colorText(BRed, centered("Invalid Path.", termWidth()));
        return "";
    }

    return folderPath;
}

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
    static const std::unordered_set<std::string> ignored = {
        "cmake-build-debug", ".git", "build", "cmakefiles",
        ".vscode", ".idea", "node_modules", "__pycache__",
        "venv", "__pycache__", "site-packages"
    };

    for (auto p : path) {
        std::string folder = toLower(p.filename().string());
        if (ignored.contains(folder) || folder.find("cmake") != std::string::npos) {
            return true;
        }
    }

    std::string filename = toLower(path.filename().string());
    if (filename == "activate_this.py") return true;

    return false;
}

void CodeCounter::execute(const std::vector<std::string>& args) {
    (void) args;
    clearScreen();
    for (size_t i = 0; i < 9; ++i) std::cout << '\n';

    const std::vector<std::string> codeCounterInfo = {
        "Code Counter Utility",
        "",
        "Commands:",
        "  1  - Show project (only C++) line count",
        "  2  - Show project Language line count",
        "",
        "Supported Languages:",
        "  C++, C#, Java, Python, Go, Rust, PHP, Assembly,",
        "  JavaScript, TypeScript, Swift, Kotlin, Ruby",
        "",
        "Navigation:",
        "  q, quit - go back to main menu"
    };
    printBox(codeCounterInfo);

    std::string input;
    while (true) {
        std::cout << "\n\n" << colorText(BWhite, centered("Enter command: ", termWidth()));
        std::cin >> input;

        if (input == "q" || input == "quit") return;

        try {
            switch (std::stoi(input)) {
                case 1: getFolderStats(); break;
                case 2: getLangStats(); break;
                default: std::cout << '\n' << colorText(BRed, centered("Wrong input!\n", termWidth())); continue;
            }
        } catch (const std::invalid_argument&) {
            std::cout << '\n' << colorText(BRed, centered("Please enter a number!\n", termWidth()));
        }
    }
}

void CodeCounter::getFolderStats() const {
    const std::string folderPath = inputFolder();
    if (folderPath.empty()) return;

    std::vector<FileStats> headers;
    std::vector<FileStats> sources;

    size_t totalHeadersLine = 0;
    size_t totalSourcesLine = 0;

    for (auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (!entry.is_regular_file() || isIgnorePath(entry.path())) continue;

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

void CodeCounter::getLangStats() {
    initMap();
    const std::string folderPath = inputFolder();
    if (folderPath.empty()) return;

    std::map<std::string, std::vector<FileStats>> filesByLang;
    std::map<std::string, size_t> totalsByLang;

    for (auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (!entry.is_regular_file() || isIgnorePath(entry.path())) continue;

        auto path = entry.path();
        auto extension = path.extension().string();

        for (const auto& [langKey, config] : languageMap) {
            if (std::find(config.extension.begin(), config.extension.end(), extension) != config.extension.end()) {
                const size_t lines = countLine(path);
                filesByLang[langKey].push_back({path.filename().string(), lines});
                totalsByLang[langKey] += lines;
            }
        }
    }

    std::cout << '\n';
    printByLanguage(filesByLang, totalsByLang, languageMap);
}

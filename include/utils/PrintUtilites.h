//
// Created by Marat on 16.08.25.
//

#pragma once
#include "Structs.h"
#include <map>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <valarray>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

inline void clearScreen() {
    std::cout << "\033[2J\033[H";
}

inline int termWidth() {
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) return ws.ws_col;
    return 80;
}

inline std::string centered(const std::string& s, const int width) {
    if (static_cast<int>(s.size()) >= width) return s;
    const int pad = (width - static_cast<int>(s.size())) / 2;
    return std::string(pad, ' ') + s;
}

inline std::string toLower(std::string s) {
    std::ranges::transform(s, s.begin(),
                           [](const unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}

inline void printBox(const std::vector<std::string>& lines) {
    const int w = termWidth();

    size_t maxLen = 0;
    for (auto& s : lines) {
        maxLen = std::max(maxLen, s.size());
    }

    const int inner = static_cast<int>(maxLen);
    const int total = inner + 4;
    const int left = std::max(0, (w - total) / 2);

    auto repeat = [](const std::string_view s, const int n) {
        std::string out;
        out.reserve(s.size() * n);
        for (int i = 0; i < n; i++) out += s;
        return out;
    };

    auto printLine = [&](const std::string& s) {
        std::cout << std::string(left, ' ')
                  << colorText(BWhite, "│ ")
                  << colorText(BWhite, s)
                  << std::string(inner - s.size(), ' ')
                  << colorText(BWhite, " │")
                  << '\n';
    };

    std::cout << std::string(left, ' ')
              << colorText(BWhite, "┌")
              << colorText(BWhite, repeat("─", inner + 2))
              << colorText(BWhite, "┐\n");

    for (auto& s : lines) printLine(s);
    std::cout << std::string(left, ' ')
              << colorText(BWhite, "└")
              << colorText(BWhite, repeat("─", inner + 2))
              << colorText(BWhite, "┘\n");
}

inline void printTable(const std::vector<FileStats>& headers,
                       const std::vector<FileStats>& sources,
                       const size_t totalHeaders, const size_t totalSources)
{
    const int headerColWidth = std::max(7, static_cast<int>(std::max_element(headers.begin(), headers.end(),
                                                                             [](auto &a, auto &b) { return a.name.size() < b.name.size(); })->name.size()) + 5 + 6);
    const int sourceColWidth = std::max(7, static_cast<int>(std::max_element(sources.begin(), sources.end(),
                                                                             [](auto &a, auto &b) { return a.name.size() < b.name.size(); })->name.size()) + 5 + 6);
    const int tableWidth = headerColWidth + sourceColWidth + 3 + 2;
    const int leftPadding = std::max(0, (termWidth() - tableWidth) / 2);

    auto repeat = [](const std::string& s, const int n){
        std::string out;
        for (int i = 0; i < n; ++i) out += s;
        return out;
    };

    auto pad = [repeat](const std::string &s, const int w) {
        const int contentWidth = std::max(0, w - 2);
        const std::string trimmed = s.substr(0, contentWidth);
        const int rightPad = contentWidth - trimmed.size();
        return " " + trimmed + std::string(rightPad, ' ') + " ";
    };

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "┌") << colorText(BWhite, repeat("─", headerColWidth))
              << colorText(BWhite, "─┬─") << colorText(BWhite, repeat("─", sourceColWidth))
              << colorText(BWhite, "┐\n");

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "│") << colorText(BWhite, pad("Headers", headerColWidth))
              << colorText(BWhite, " │") << colorText(BWhite, pad("Sources", sourceColWidth))
              << colorText(BWhite, " │\n");

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "├") << colorText(BWhite, repeat("─", headerColWidth))
              << colorText(BWhite, "─┼─") << colorText(BWhite, repeat("─", sourceColWidth))
              << colorText(BWhite, "┤\n");

    const size_t rows = std::max(headers.size(), sources.size());
    for (size_t i = 0; i < rows; ++i) {
        std::string h = (i < headers.size()) ? headers[i].name + " - " + std::to_string(headers[i].lines) : "";
        std::string s = (i < sources.size()) ? sources[i].name + " - " + std::to_string(sources[i].lines) : "";
        if (h.empty() && s.empty()) continue;

        std::cout << std::string(leftPadding, ' ')
                  << colorText(BWhite, "│") << colorText(BWhite, pad(h, headerColWidth))
                  << colorText(BWhite, " │") << colorText(BWhite, pad(s, sourceColWidth))
                  << colorText(BWhite, " │\n");
    }

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "├") << colorText(BWhite, repeat("─", headerColWidth))
              << colorText(BWhite, "─┼─") << colorText(BWhite, repeat("─", sourceColWidth))
              << colorText(BWhite, "┤\n");

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "│") << colorText(BWhite, pad("Total(.h) lines - " + std::to_string(totalHeaders), headerColWidth))
              << colorText(BWhite, " │") << colorText(BWhite, pad("Total(.cpp) lines - " + std::to_string(totalSources), sourceColWidth))
              << colorText(BWhite, " │\n");

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "├") << colorText(BWhite, repeat("─", headerColWidth))
              << colorText(BWhite, "─┼─") << colorText(BWhite, repeat("─", sourceColWidth))
              << colorText(BWhite, "┤\n");

    const size_t totalLines = totalHeaders + totalSources;
    const std::string totalStr = "Total lines - " + std::to_string(totalLines);
    const int innerWidth = headerColWidth + sourceColWidth + 3;
    const int leftInnerPadding = (innerWidth - totalStr.size()) / 2;
    const int rightInnerPadding = innerWidth - totalStr.size() - leftInnerPadding;

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "│")
              << colorText(BWhite, std::string(leftInnerPadding, ' '))
              << colorText(BWhite, totalStr)
              << colorText(BWhite, std::string(rightInnerPadding, ' '))
              << colorText(BWhite, "│\n");

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "└") << colorText(BWhite, repeat("─", headerColWidth))
              << colorText(BWhite, "─┴─") << colorText(BWhite, repeat("─", sourceColWidth))
              << colorText(BWhite, "┘\n");
}

inline void printByLanguage(const std::map<std::string, std::vector<FileStats>>& filesByLang,
                            const std::map<std::string, size_t>& totalsByLang,
                            const std::map<std::string, LangConfig>& languageMap) {

    for (const auto& [langKey, files] : filesByLang) {

        std::string langName = langKey;
        auto itMap = languageMap.find(langKey);
        if (itMap != languageMap.end()) langName = itMap->second.name;

        std::vector<std::string> lines;
        lines.push_back("Language: " + langName);

        std::map<std::string, std::vector<FileStats>> byExt;
        for (const auto& f : files) {
            auto ext = fs::path(f.name).extension().string();
            if (ext.empty()) ext = "[no-ext]";
            byExt[ext].push_back(f);
        }

        for (const auto& [ext, extFiles] : byExt) {
            lines.push_back(" ├ " + ext);
            for (const auto& f : extFiles) {
                lines.push_back(" │   " + f.name + " - " + std::to_string(f.lines));
            }
        }

        size_t langTotal = 0;
        auto it = totalsByLang.find(langKey);
        if (it != totalsByLang.end()) langTotal = it->second;
        lines.push_back(" └ Overall: " + std::to_string(langTotal));

        int maxWidth = 0;
        for (const auto& l : lines) maxWidth = std::max(maxWidth, static_cast<int>(l.size()));
        const int leftPadding = std::max(0, (termWidth() - maxWidth) / 2);

        for (const auto& l : lines) {
            std::cout << std::string(leftPadding, ' ') << colorText(BWhite, l) << "\n";
        }
        std::cout << "\n";
    }
}

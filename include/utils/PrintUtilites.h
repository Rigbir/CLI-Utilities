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

inline size_t utf8VisibleLenNoANSI(const std::string_view& s) {
    size_t n = 0;
    for (size_t i = 0; i < s.size();) {
        const unsigned char c = static_cast<unsigned char>(s[i]);

        if (c == 0x1B) {
            ++i;
            if (i < s.size()) {
                const unsigned char t = static_cast<unsigned char>(s[i]);
                if (t == '[') {
                    ++i;
                    while (i < s.size()) {
                        const unsigned char d = static_cast<unsigned char>(s[i++]);
                        if (d >= 0x40 && d <= 0x7E) break;
                    }
                    continue;
                } else if (t == ']') {
                    ++i;
                    while (i < s.size()) {
                        const unsigned char d = static_cast<unsigned char>(s[i]);
                        if (d == 0x07) { ++i; break; }
                        if (d == 0x1B && i + 1 < s.size() && s[i + 1] == '\\') { i += 2; break; }
                        ++i;
                    }
                    continue;
                } else {
                    ++i;
                    continue;
                }
            }
            continue;
        }

        if ((c & 0x80) == 0x00) {
            ++n; ++i;
        } else if ((c & 0xE0) == 0xC0) {
            ++n; i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            ++n; i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            ++n; i += 4;
        } else {
            ++i;
        }
    }
    return n;
}

inline void printCenteredBlock(const std::vector<std::string>& colored,
                               const std::vector<std::string>& plain,
                               const int termW) {

    size_t maxLen = 0;
    for (const auto& p : plain) {
        maxLen = std::max(maxLen, utf8VisibleLenNoANSI(p));
    }
    const int leftPad = std::max(0, (termW - static_cast<int>(maxLen)) / 2);
    const std::string margin(leftPad, ' ');

    for (const auto& line : colored) {
        std::cout << margin << line << "\n\n";
    }
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

inline void printBoxes(const std::vector<std::vector<std::string>>& tables) {
    const int w = termWidth();

    std::vector<int> widths;
    widths.reserve(tables.size());

    size_t maxHeight = 0;
    for (auto& lines : tables) {
        size_t maxLen = 0;
        for (auto& s : lines) maxLen = std::max(maxLen, s.size());
        widths.push_back(static_cast<int>(maxLen));
        maxHeight = std::max(maxHeight, lines.size());
    }

    int totalWidth = 0;
    for (const auto wBox : widths) totalWidth += wBox + 4;
    totalWidth += (static_cast<int>(tables.size()) - 1) * 2;

    const int left = std::max(0, (w - totalWidth) / 2);

    auto repeat = [](const std::string_view s, const int n) {
        std::string out;
        out.reserve(s.size() * n);
        for (int i = 0; i < n; i++) out += s;
        return out;
    };

    std::cout << std::string(left, ' ');
    for (size_t i = 0; i < tables.size(); i++) {
        std::cout << colorText(BWhite, "┌")
                  << colorText(BWhite, repeat("─", widths[i] + 2))
                  << colorText(BWhite, "┐");
        if (i + 1 < tables.size()) std::cout << "  ";
    }
    std::cout << "\n";

    for (size_t row = 0; row < maxHeight; row++) {
        std::cout << std::string(left, ' ');
        for (size_t i = 0; i < tables.size(); i++) {
            std::string text;
            if (row < tables[i].size()) text = tables[i][row];

            std::cout << colorText(BWhite, "│ ")
                      << colorText(BWhite, text)
                      << std::string(widths[i] - text.size(), ' ')
                      << colorText(BWhite, " │");

            if (i + 1 < tables.size()) std::cout << "  ";
        }
        std::cout << "\n";
    }

    std::cout << std::string(left, ' ');
    for (size_t i = 0; i < tables.size(); i++) {
        std::cout << colorText(BWhite, "└")
                  << colorText(BWhite, repeat("─", widths[i] + 2))
                  << colorText(BWhite, "┘");
        if (i + 1 < tables.size()) std::cout << "  ";
    }
    std::cout << "\n";
}

inline void printTable(const std::vector<FileStats>& headers,
                       const std::vector<FileStats>& sources,
                       const size_t totalHeaders, const size_t totalSources)
{
    const int headerColWidth = std::max(7, static_cast<int>(std::max_element(headers.begin(), headers.end(),
                                                                             [](auto &a, auto &b) { return a.name.size() < b.name.size(); })->name.size()) + 5 + 7);
    const int sourceColWidth = std::max(7, static_cast<int>(std::max_element(sources.begin(), sources.end(),
                                                                             [](auto &a, auto &b) { return a.name.size() < b.name.size(); })->name.size()) + 5 + 7);

    const int tableWidth = headerColWidth + sourceColWidth + 3 + 2;
    const int leftPadding = std::max(0, (termWidth() - tableWidth) / 2);

    auto repeat = [](const std::string& s, const int n){
        std::string out;
        for (int i = 0; i < n; ++i) out += s;
        return out;
    };

    auto pad = [](const std::string &s, const int w) {
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
        lines.push_back(" └ Overall lines: " + std::to_string(langTotal));

        int maxWidth = 0;
        for (const auto& l : lines) maxWidth = std::max(maxWidth, static_cast<int>(l.size()));
        const int leftPadding = std::max(0, (termWidth() - maxWidth) / 2);

        for (const auto& l : lines) {
            std::cout << std::string(leftPadding, ' ') << colorText(BWhite, l) << "\n";
        }
        std::cout << "\n";
    }
}

inline int utf8Length(const std::string& s) {
    int count = 0;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = s[i];
        if      ((c & 0x80) == 0) i += 1;
        else if ((c & 0xE0) == 0xC0) i += 2;
        else if ((c & 0xF0) == 0xE0) i += 3;
        else if ((c & 0xF8) == 0xF0) i += 4;
        else i += 1;
        count++;
    }
    return count;
}

inline std::string pad(const std::string& s, int w, bool rightAlign) {
    const int len = utf8Length(s);
    if (len >= w) return s;
    const int spaces = w - len;
    if (rightAlign)
        return std::string(spaces, ' ') + s;
    else
        return s + std::string(spaces, ' ');
}

inline void printProcessTable(const std::vector<std::vector<std::string>>& rows) {
    if (rows.empty()) return;

    size_t colCount = 0;
    for (const auto& r : rows) colCount = std::max(colCount, r.size());
    if (colCount == 0) return;

    std::vector<int> colWidths(colCount, 0);
    for (const auto& r : rows) {
        for (size_t c = 0; c < colCount; ++c) {
            const std::string cell = (c < r.size()) ? r[c] : std::string();
            int w = utf8Length(cell);
            if (w > colWidths[c]) colWidths[c] = w;
        }
    }
    for (auto& w : colWidths) w = std::max(w, 3);

    auto repeat = [](const std::string& s, int n) {
        std::string out;
        for (int i = 0; i < n; i++) out += s;
        return out;
    };

    const int innerWidth = [&]{
        int sum = 0;
        for (auto w : colWidths) sum += w + 2;
        return sum + (colCount - 1);
    }();
    const int leftPadding = std::max(0, (termWidth() - (innerWidth + 2)) / 2);

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "┌");
    for (size_t c = 0; c < colCount; ++c) {
        std::cout << colorText(BWhite, repeat("─", colWidths[c] + 2));
        if (c + 1 < colCount) std::cout << colorText(BWhite, "┬");
    }
    std::cout << colorText(BWhite, "┐\n");

    std::cout << std::string(leftPadding, ' ') << colorText(BWhite, "│");
    for (size_t c = 0; c < colCount; ++c) {
        const std::string cell = (c < rows[0].size()) ? rows[0][c] : "";
        std::string padded = pad(cell, colWidths[c], false);
        std::cout << colorText(BWhite, " ")
                  << colorText(BWhite, padded)
                  << colorText(BWhite, " ");
        if (c + 1 < colCount) std::cout << colorText(BWhite, "│");
    }
    std::cout << colorText(BWhite, "│\n");

    std::cout << std::string(leftPadding, ' ') << colorText(BWhite, "├");
    for (size_t c = 0; c < colCount; ++c) {
        std::cout << colorText(BWhite, repeat("─", colWidths[c] + 2));
        if (c + 1 < colCount) std::cout << colorText(BWhite, "┼");
    }
    std::cout << colorText(BWhite, "┤\n");

    for (size_t r = 1; r < rows.size(); ++r) {
        std::cout << std::string(leftPadding, ' ') << colorText(BWhite, "│");
        for (size_t c = 0; c < colCount; ++c) {
            const std::string cell = (c < rows[r].size()) ? rows[r][c] : "";
            std::string padded = pad(cell, colWidths[c], false);
            std::cout << colorText(BWhite, " ")
                      << colorText(BWhite, padded)
                      << colorText(BWhite, " ");
            if (c + 1 < colCount) std::cout << colorText(BWhite, "│");
        }
        std::cout << colorText(BWhite, "│\n");
    }

    std::cout << std::string(leftPadding, ' ')
              << colorText(BWhite, "└");
    for (size_t c = 0; c < colCount; ++c) {
        std::cout << colorText(BWhite, repeat("─", colWidths[c] + 2));
        if (c + 1 < colCount) std::cout << colorText(BWhite, "┴");
    }
    std::cout << colorText(BWhite, "┘\n");
}

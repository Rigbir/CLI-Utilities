//
// Created by Marat on 16.08.25.
//

#pragma once
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <valarray>

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
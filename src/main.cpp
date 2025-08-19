#include "Command.h"
#include "BatteryMonitor.h"
#include "Cleaner.h"
#include "DeviceWatcher.h"
#include "WifiMonitor.h"
#include "CodeCounter.h"
#include "SystemInfo.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <csignal>
#include <limits>

static volatile sig_atomic_t g_running = 1;

static void handleSigInt(int) {
    g_running = 0;
    if (const auto rl = CFRunLoopGetMain()) CFRunLoopStop(rl);
    std::cout << "\n" << colorText(BYellow, "[Ctrl+C] Requested exit...\n");
}

static void waitAnyKey(const std::string& prompt = "Press Enter to continue...") {
    std::cout << "\n" << colorText(BYellow, centered(prompt, termWidth()));
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static std::vector<std::string> splitArgs(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    for (const char c : line) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

static void showWelcome(const std::map<std::string, std::unique_ptr<Command>>& commands) {
    clearScreen();
    const int w = termWidth();
    for (size_t i = 0; i < 7; ++i) {
        std::cout << '\n';
    }

    const std::string lineChar = "─";
    std::string lineFull;
    for (int i = 0; i < w / 2; ++i) lineFull += lineChar;
    const int padding = std::abs(w - static_cast<int>(lineFull.size())) / 2;

    std::cout << colorText(BCyan,  centered("CLI Utilities", w)) << "\n\n";

    std::cout << std::string(padding, ' ')
               << colorText(BPurple, lineFull) << "\n\n";

    std::cout << colorText(BBlue, centered("Available commands:", w)) << "\n\n";

    std::string line;
    for (const auto& [name, _] : commands) {
        line += name + "   ";
    }
    if (!line.empty()) line.pop_back();
    std::cout << colorText(BGreen, centered(line, w)) << "\n\n";
    std::cout << colorText(BYellow, centered("[h] help   [q] quit", w)) << "\n\n";
    std::cout << std::string(padding, ' ')
              << colorText(BPurple, lineFull) << "\n\n";
}

static void showHelp() {
    clearScreen();
    const int w = termWidth();
    for (size_t i = 0; i < 5; ++i) std::cout << '\n';

    const std::vector<std::string> lines = {
        "CLI Utilities - mini TUI",
        "",
        "Commands:",
        "  battery      - Battery monitoring tools",
        "  cleaner      - Cleaner utilities",
        "  codecounter  - Project Line Counter utilities",
        "  wifi         - Wi-Fi monitoring",
        "  device       - USB / Audio / Display watcher",
        "",
        "Usage:",
        "  ./app <command>",
        "  ./app # open interactive menu",
        "",
        "Navigation:",
        "  h, help  - show this help",
        "  q, quit  - exit the app",
        "",
        "Examples:",
        "  ./app battery",
        "  ./app cleaner",
        "  ./app codecounter",
        "  ./app wifi",
        "  ./app device"
    };

    printBox(lines);
    std::cout << "\n";
    std::cout << colorText(BYellow, centered("Press Enter to go back", w)) << "\n";
}

int main(const int argc, const char* argv[]) {
    std::signal(SIGINT, handleSigInt);

    std::map<std::string, std::unique_ptr<Command>> commands;
    commands["battery"] = std::make_unique<BatteryMonitor>();
    commands["cleaner"] = std::make_unique<Cleaner>();
    commands["wifi"]    = std::make_unique<WifiMonitor>();
    commands["device"]  = std::make_unique<DeviceWatcher>();
    commands["codecounter"] = std::make_unique<CodeCounter>();
    commands["system"] = std::make_unique<SystemInfo>();

    if (argc > 1) {
        const std::string cmd = argv[1];
        const auto it = commands.find(cmd);
        if (it == commands.end()) {
            std::cerr << colorText(BRed, "\nUnknown command: " + cmd + "\n");
            return 1;
        }
        const std::vector<std::string> args(argv + 2, argv + argc);
        it->second->execute(args);
        return 0;
    }

    while (g_running) {
        showWelcome(commands);

        std::cout << colorText(C_White, centered("Enter command (or 'h' for help, 'q' to quit): ", termWidth()));
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            if (!std::cin.good()) {
                std::cin.clear();
            }
            continue;
        }

        auto tokens = splitArgs(line);
        if (tokens.empty()) continue;

        std::string cmd = toLower(tokens[0]);
        if (cmd == "q" || cmd == "quit" || cmd == ":q") break;

        if (cmd == "h" || cmd == "help" || cmd == ":h") {
            showHelp();
            waitAnyKey();
            continue;
        }

        if (auto it = commands.find(cmd); it != commands.end()) {
            std::vector<std::string> args;
            if (tokens.size() > 1) {
                args.assign(tokens.begin() + 1, tokens.end());
            }
            it->second->execute(args);
            continue;
        }

        std::cout << '\n' << colorText(BRed, centered("Unknown command. Press Enter...", termWidth())) << "\n";
        waitAnyKey();
    }

    clearScreen();
    std::cout << "\n\n" << colorText(BGreen, centered("Goodbye!", termWidth())) << "\n\n";
    return 0;
}

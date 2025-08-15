#include "Command.h"
#include "BatteryMonitor.h"
#include "Cleaner.h"
#include "DeviceWatcher.h"
#include "WifiMonitor.h"

#include <iostream>
#include <string>
#include <memory>
#include <map>

int main(int argc, const char* argv[]) {
    std::map<std::string, std::unique_ptr<Command>> commands;
    commands["battery"] = std::make_unique<BatteryMonitor>();
    commands["cleaner"] = std::make_unique<Cleaner>();
    commands["wifi"] = std::make_unique<WifiMonitor>();
    commands["device"] = std::make_unique<DeviceWatcher>();

    if (argc < 2) {
        std::cerr << colorText(BRed, "\nUsage: mytool <command>\n");
        return 1;
    }

    const std::string cmd = argv[1];
    if (const auto it = commands.find(cmd); it != commands.end()) {
        const std::vector<std::string> args(argv + 2, argv + argc);
        it->second->execute(args);
    } else {
        std::cerr << colorText(BRed, "\nUnknown command: " + cmd) << '\n';
        return 1;
    }

    return 0;
}
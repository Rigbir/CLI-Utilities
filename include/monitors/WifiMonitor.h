//
// Created by Marat on 13.08.25.
//

#pragma once
#include "IWifiMonitor.h"

class WifiMonitor final : public IWifiMonitor {
public:
    void execute(const std::vector<std::string>& args) override;
    void showCurrentConnections() const override;
    void listConnections() const override;
    void monitorConnections() const override;

private:
    static std::string trim(const std::string& s);
    static std::string runCommandWdutil(const std::string& cmd);
    static std::string runCommandSystem(const std::string& cmd,
                                        const std::string& startMarker,
                                        const std::string& endMarker);

    static void runLiveMonitor(const std::string& command,
                               const std::string& startMarker,
                               const std::string& endMarker,
                               const std::string& message);

    static void printOutput(const std::string& result);
};
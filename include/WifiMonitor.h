//
// Created by Marat on 13.08.25.
//

#pragma once
#include "IWifiMonitor.h"

class WifiMonitor final : public IWifiMonitor {
public:
    void execute(const std::vector<std::string>& args) override;
    void showCurrentConnections() const override;
    [[noreturn]] void listConnections() const override;
    [[noreturn]] void monitorConnections() const override;

private:
    static std::string trim(const std::string& s);
    static std::string runCommandWdutil(const std::string& cmd);
    static std::string runCommandSystem(const std::string& cmd
                                      , const std::string& startMarker
                                      , const std::string& endMarker);
};
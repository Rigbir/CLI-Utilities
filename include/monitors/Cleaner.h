//
// Created by Marat on 13.08.25.
//

#pragma once
#include "ICleaner.h"
#include <map>

class Cleaner final : public ICleaner {
public:
    void execute(const std::vector<std::string>& args) override;
    void getAllInfo() override;
    std::string getStats(const std::string& path) const override;
    void removeFile() override;
    void largeDirectory() override;

private:
    std::map<std::string, std::map<std::string, std::string>> allEntries;
    std::map<std::string, std::map<std::string, std::string>> removeEntries;
    [[nodiscard]] static std::string getFolder();
    [[nodiscard]] static std::string resolveFolderPath(const std::string& key);
    [[nodiscard]] static std::string shortPath(const std::string& path);
    static bool confirmation(const std::string& text);
    void printFileInFolder(const std::string& folder);
};
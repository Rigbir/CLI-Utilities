//
// Created by Marat on 13.08.25.
//

#pragma once
#include "ICleaner.h"
#include <map>

class Cleaner final : public ICleaner {
public:
    [[noreturn]] void execute(const std::vector<std::string>& args) override;
    [[noreturn]] void getAllInfo() override;
    [[nodiscard]] std::string getStats(const std::string& path) const override;
    [[noreturn]] void removeFile(const std::string& folder) override;

private:
    std::map<std::string, std::string> allEntries;
    [[nodiscard]] static std::string getFolder();
    [[nodiscard]] static std::string resolveFolderPath(const std::string& key);
    [[nodiscard]] static std::string lower(std::string& word);
    [[noreturn]] void printFileInFolder(const std::string& folder) const;
};
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
    void removeFile(const std::string& folder) override;

private:
    std::map<std::string, std::string> allEntries;
    static std::string getFolder();
    static std::string resolveFolderPath(const std::string& key);
    static std::string lower(std::string& word);
    void printFileInFolder(const std::string& folder);
};
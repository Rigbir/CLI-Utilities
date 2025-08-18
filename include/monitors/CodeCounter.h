//
// Created by Marat on 17.08.25.
//

#pragma once
#include "Structs.h"
#include "ICodeCounter.h"
#include <filesystem>
#include <map>

class CodeCounter final : public ICodeCounter {
public:
    void execute(const std::vector<std::string>& args) override;
    void getFolderStats() const override;
    void getLangStats() override;

private:
    std::map<std::string, LangConfig> languageMap;

    void initMap();
    [[nodiscard]] static std::string inputFolder();
    [[nodiscard]] static size_t countLine(const std::string& folderPath);
    [[nodiscard]] static bool isIgnorePath(const std::filesystem::path& path);
};

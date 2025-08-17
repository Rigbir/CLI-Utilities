//
// Created by Marat on 17.08.25.
//

#pragma once
#include <filesystem>
#include "Structs.h"
#include "ICodeCounter.h"

class CodeCounter final : public ICodeCounter {
public:
    void execute(const std::vector<std::string>& args) override;
    void getFolderStats() const override;

private:
    [[nodiscard]] static size_t countLine(const std::string& folderPath);
    [[nodiscard]] static bool isIgnorePath(const std::filesystem::path& path);
};

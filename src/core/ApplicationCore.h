#pragma once

#include <filesystem>
#include <string>

namespace blaze {

class ApplicationCore final {
public:
    explicit ApplicationCore(std::filesystem::path workspaceRoot);

    [[nodiscard]] std::string workspaceInfoJson() const;
    [[nodiscard]] std::string readFileJson(const std::string& relativePath) const;
    [[nodiscard]] std::string writeFileJson(
        const std::string& relativePath,
        const std::string& contents
    ) const;

private:
    [[nodiscard]] std::filesystem::path resolveWorkspacePath(
        const std::string& relativePath
    ) const;

    [[nodiscard]] static std::string jsonEscape(const std::string& value);
    [[nodiscard]] static std::string jsonString(const std::string& value);

    std::filesystem::path m_workspaceRoot;
};

} // namespace blaze

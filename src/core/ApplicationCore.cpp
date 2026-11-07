#include "core/ApplicationCore.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace blaze {
namespace {

bool isInside(const std::filesystem::path& root, const std::filesystem::path& candidate) {
    auto rootIt = root.begin();
    auto candidateIt = candidate.begin();
    for (; rootIt != root.end(); ++rootIt, ++candidateIt) {
        if (candidateIt == candidate.end() || *rootIt != *candidateIt) {
            return false;
        }
    }
    return true;
}

} // namespace

ApplicationCore::ApplicationCore(std::filesystem::path workspaceRoot) {
    std::error_code error;
    m_workspaceRoot = std::filesystem::weakly_canonical(workspaceRoot, error);
    if (error || m_workspaceRoot.empty()) {
        throw std::runtime_error("The workspace directory could not be resolved.");
    }
    if (!std::filesystem::is_directory(m_workspaceRoot)) {
        throw std::runtime_error("The workspace path is not a directory.");
    }
}

std::string ApplicationCore::workspaceInfoJson() const {
    std::ostringstream json;
    json << "{\"ok\":true,\"name\":"
         << jsonString(m_workspaceRoot.filename().string())
         << ",\"root\":" << jsonString(m_workspaceRoot.string())
         << ",\"files\":[";

    bool first = true;
    std::size_t count = 0;
    std::error_code error;
    for (std::filesystem::recursive_directory_iterator it(
             m_workspaceRoot,
             std::filesystem::directory_options::skip_permission_denied,
             error), end;
         it != end && count < 400; it.increment(error)) {
        if (error) {
            error.clear();
            continue;
        }

        const auto relative = std::filesystem::relative(it->path(), m_workspaceRoot, error);
        if (error) {
            error.clear();
            continue;
        }

        const auto relativeText = relative.generic_string();
        const auto firstPart = *relative.begin();
        if (firstPart == ".git" || firstPart == "build" ||
            firstPart == "build-release" || firstPart == ".idea" ||
            firstPart == "node_modules") {
            if (it->is_directory(error)) {
                it.disable_recursion_pending();
            }
            continue;
        }

        if (it->is_regular_file(error)) {
            if (!first) json << ',';
            first = false;
            json << "{\"path\":" << jsonString(relativeText)
                 << ",\"name\":" << jsonString(it->path().filename().string())
                 << "}";
            ++count;
        }
    }

    json << "]}";
    return json.str();
}

std::string ApplicationCore::readFileJson(const std::string& relativePath) const {
    try {
        const auto path = resolveWorkspacePath(relativePath);
        if (!std::filesystem::is_regular_file(path)) {
            return "{\"ok\":false,\"error\":\"The requested path is not a file.\"}";
        }

        std::ifstream input(path, std::ios::binary);
        if (!input) {
            return "{\"ok\":false,\"error\":\"The file could not be opened.\"}";
        }

        const std::string contents(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>()
        );
        return "{\"ok\":true,\"path\":" + jsonString(relativePath) +
               ",\"content\":" + jsonString(contents) + "}";
    } catch (const std::exception& error) {
        return "{\"ok\":false,\"error\":" + jsonString(error.what()) + "}";
    }
}

std::string ApplicationCore::writeFileJson(
    const std::string& relativePath,
    const std::string& contents
) const {
    try {
        const auto path = resolveWorkspacePath(relativePath);
        std::filesystem::create_directories(path.parent_path());

        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        if (!output) {
            return "{\"ok\":false,\"error\":\"The file could not be saved.\"}";
        }

        output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        if (!output.good()) {
            return "{\"ok\":false,\"error\":\"Writing the file failed.\"}";
        }

        return "{\"ok\":true,\"path\":" + jsonString(relativePath) + "}";
    } catch (const std::exception& error) {
        return "{\"ok\":false,\"error\":" + jsonString(error.what()) + "}";
    }
}

std::filesystem::path ApplicationCore::resolveWorkspacePath(
    const std::string& relativePath
) const {
    if (relativePath.empty()) {
        throw std::runtime_error("A relative file path is required.");
    }

    const std::filesystem::path requested{relativePath};
    if (requested.is_absolute()) {
        throw std::runtime_error("Absolute paths are not allowed.");
    }

    std::error_code error;
    const auto resolved = std::filesystem::weakly_canonical(
        m_workspaceRoot / requested, error
    );
    if (error || !isInside(m_workspaceRoot, resolved)) {
        throw std::runtime_error("The path must remain inside the current workspace.");
    }
    return resolved;
}

std::string ApplicationCore::jsonEscape(const std::string& value) {
    std::ostringstream escaped;
    for (const unsigned char character : value) {
        switch (character) {
        case '"': escaped << "\\\""; break;
        case '\\': escaped << "\\\\"; break;
        case '\n': escaped << "\\n"; break;
        case '\r': escaped << "\\r"; break;
        case '\t': escaped << "\\t"; break;
        default:
            if (character < 0x20) {
                constexpr char hex[] = "0123456789abcdef";
                escaped << "\\u00" << hex[character >> 4] << hex[character & 0x0f];
            } else {
                escaped << static_cast<char>(character);
            }
        }
    }
    return escaped.str();
}

std::string ApplicationCore::jsonString(const std::string& value) {
    return "\"" + jsonEscape(value) + "\"";
}

} // namespace blaze

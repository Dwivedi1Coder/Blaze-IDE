#pragma once

#include "core/ApplicationCore.h"
#include "webview/webview.h"

#include <filesystem>
#include <string>
#include <vector>

namespace blaze {

class NativeBridge final {
public:
    NativeBridge(webview_t window, ApplicationCore& core);

    void registerBindings();
    [[nodiscard]] static std::string toFileUri(const std::filesystem::path& path);

private:
    struct Context {
        webview_t window;
        ApplicationCore* core;
    };

    static void onGetWorkspace(const char* id, const char* request, void* argument);
    static void onReadFile(const char* id, const char* request, void* argument);
    static void onWriteFile(const char* id, const char* request, void* argument);

    static void respond(Context& context, const char* id, const std::string& json);
    static std::vector<std::string> parseStringArguments(const char* request);

    Context m_context;
};

} // namespace blaze

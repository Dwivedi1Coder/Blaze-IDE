#include "core/ApplicationCore.h"
#include "core/NativeBridge.h"

#include <filesystem>
#include <iostream>
#include <string>

#include "webview/webview.h"

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
    webview_t window = webview_create(1, nullptr);
    if (window == nullptr) {
        std::cerr << "Blaze-IDE could not initialize the native WebView runtime.\n";
        return 1;
    }

    try {
#ifdef BLAZE_DEFAULT_WORKSPACE
        const std::filesystem::path workspace{BLAZE_DEFAULT_WORKSPACE};
#else
        const std::filesystem::path workspace = std::filesystem::current_path();
#endif
        blaze::ApplicationCore core{workspace};
        blaze::NativeBridge bridge{window, core};

        webview_set_title(window, "Blaze-IDE");
        webview_set_size(window, 1440, 920, WEBVIEW_HINT_NONE);
        bridge.registerBindings();

#ifdef BLAZE_UI_DIR
        const auto entry = std::filesystem::path{BLAZE_UI_DIR} / "index.html";
#else
        const auto entry = std::filesystem::current_path() / "ui" / "index.html";
#endif
        const std::string uri = blaze::NativeBridge::toFileUri(entry);
        webview_navigate(window, uri.c_str());
        webview_run(window);
    } catch (const std::exception& error) {
        std::cerr << "Blaze-IDE startup failed: " << error.what() << '\n';
        webview_destroy(window);
        return 1;
    }

    webview_destroy(window);
    return 0;
}

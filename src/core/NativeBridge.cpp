#include "core/NativeBridge.h"

#include <cctype>
#include <stdexcept>
#include <utility>

namespace blaze {

NativeBridge::NativeBridge(webview_t window, ApplicationCore& core)
    : m_context{window, &core} {}

void NativeBridge::registerBindings() {
    webview_bind(m_context.window, "nativeGetWorkspace", &NativeBridge::onGetWorkspace, &m_context);
    webview_bind(m_context.window, "nativeReadFile", &NativeBridge::onReadFile, &m_context);
    webview_bind(m_context.window, "nativeWriteFile", &NativeBridge::onWriteFile, &m_context);
}

void NativeBridge::onGetWorkspace(const char* id, const char*, void* argument) {
    auto& context = *static_cast<Context*>(argument);
    respond(context, id, context.core->workspaceInfoJson());
}

void NativeBridge::onReadFile(const char* id, const char* request, void* argument) {
    auto& context = *static_cast<Context*>(argument);
    const auto args = parseStringArguments(request);
    if (args.empty()) {
        respond(context, id, "{\"ok\":false,\"error\":\"Missing relative file path.\"}");
        return;
    }
    respond(context, id, context.core->readFileJson(args[0]));
}

void NativeBridge::onWriteFile(const char* id, const char* request, void* argument) {
    auto& context = *static_cast<Context*>(argument);
    const auto args = parseStringArguments(request);
    if (args.size() < 2) {
        respond(context, id, "{\"ok\":false,\"error\":\"Expected a path and file contents.\"}");
        return;
    }
    respond(context, id, context.core->writeFileJson(args[0], args[1]));
}

void NativeBridge::respond(Context& context, const char* id, const std::string& json) {
    webview_return(context.window, id, 0, json.c_str());
}

std::vector<std::string> NativeBridge::parseStringArguments(const char* request) {
    std::vector<std::string> values;
    if (request == nullptr) return values;

    const std::string input{request};
    std::size_t index = 0;
    while (index < input.size()) {
        if (input[index] != '"') {
            ++index;
            continue;
        }

        ++index;
        std::string value;
        while (index < input.size() && input[index] != '"') {
            char current = input[index++];
            if (current == '\\' && index < input.size()) {
                const char escaped = input[index++];
                switch (escaped) {
                case '"': value.push_back('"'); break;
                case '\\': value.push_back('\\'); break;
                case '/': value.push_back('/'); break;
                case 'n': value.push_back('\n'); break;
                case 'r': value.push_back('\r'); break;
                case 't': value.push_back('\t'); break;
                case 'b': value.push_back('\b'); break;
                case 'f': value.push_back('\f'); break;
                default: value.push_back(escaped); break;
                }
            } else {
                value.push_back(current);
            }
        }
        if (index < input.size()) ++index;
        values.push_back(std::move(value));
    }
    return values;
}

std::string NativeBridge::toFileUri(const std::filesystem::path& path) {
    const std::string native = std::filesystem::absolute(path).generic_string();
    std::string uri = native.empty() || native.front() != '/' ? "file:///" : "file://";
    for (const unsigned char character : native) {
        if (character == ' ') uri += "%20";
        else if (character == '#') uri += "%23";
        else if (character == '%') uri += "%25";
        else if (character == '?') uri += "%3F";
        else uri += static_cast<char>(character);
    }
    return uri;
}

} // namespace blaze

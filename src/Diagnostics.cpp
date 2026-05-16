#include "Diagnostics.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <windows.h>

#include "Config.h"
#include "Url.h"
#include "Version.h"

namespace {

std::atomic_bool g_debugLoggingEnabled{false};

std::wstring NowText() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &time);
    std::wostringstream out;
    out << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
    return out.str();
}

std::wstring Lowercase(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(towlower(ch));
    });
    return value;
}

bool IsQueryParamStart(const std::wstring& value, size_t keyPos) {
    return keyPos == 0 || value[keyPos - 1] == L'?' || value[keyPos - 1] == L'&';
}

void RedactQueryParam(std::wstring& value, const std::wstring& key) {
    const std::wstring needle = key + L"=";
    std::wstring lower = Lowercase(value);
    const std::wstring lowerNeedle = Lowercase(needle);
    size_t searchFrom = 0;
    while (true) {
        const size_t keyPos = lower.find(lowerNeedle, searchFrom);
        if (keyPos == std::wstring::npos) {
            break;
        }
        if (!IsQueryParamStart(value, keyPos)) {
            searchFrom = keyPos + lowerNeedle.size();
            continue;
        }
        const size_t valueStart = keyPos + needle.size();
        const size_t valueEnd = value.find(L'&', valueStart);
        value.replace(valueStart, valueEnd == std::wstring::npos ? std::wstring::npos : valueEnd - valueStart, L"<redacted>");
        lower = Lowercase(value);
        searchFrom = valueStart + 10;
    }
}

bool IsKeyBoundary(const std::wstring& value, size_t pos) {
    if (pos == 0) {
        return true;
    }
    const wchar_t previous = value[pos - 1];
    return iswspace(previous) || previous == L'"' || previous == L'\'' ||
        previous == L'{' || previous == L',' || previous == L';' || previous == L'&';
}

void RedactKeyValue(std::wstring& value, const std::wstring& key) {
    std::wstring lower = Lowercase(value);
    const std::wstring lowerKey = Lowercase(key);
    size_t searchFrom = 0;
    while (true) {
        const size_t keyPos = lower.find(lowerKey, searchFrom);
        if (keyPos == std::wstring::npos) {
            break;
        }
        if (!IsKeyBoundary(value, keyPos)) {
            searchFrom = keyPos + lowerKey.size();
            continue;
        }

        size_t separator = keyPos + key.size();
        while (separator < value.size() && iswspace(value[separator])) {
            ++separator;
        }
        if (separator >= value.size() || (value[separator] != L'=' && value[separator] != L':')) {
            searchFrom = keyPos + lowerKey.size();
            continue;
        }

        size_t valueStart = separator + 1;
        while (valueStart < value.size() && iswspace(value[valueStart])) {
            ++valueStart;
        }
        const bool quoted = valueStart < value.size() && (value[valueStart] == L'"' || value[valueStart] == L'\'');
        const bool redactToLineEnd = lowerKey.find(L"authorization") != std::wstring::npos;
        const wchar_t quote = quoted ? value[valueStart++] : 0;
        size_t valueEnd = valueStart;
        while (valueEnd < value.size()) {
            const wchar_t ch = value[valueEnd];
            if ((quoted && ch == quote) ||
                (!quoted && redactToLineEnd && (ch == L'\r' || ch == L'\n')) ||
                (!quoted && !redactToLineEnd && (iswspace(ch) || ch == L'&' || ch == L',' || ch == L';'))) {
                break;
            }
            ++valueEnd;
        }
        value.replace(valueStart, valueEnd - valueStart, L"<redacted>");
        lower = Lowercase(value);
        searchFrom = valueStart + 10;
    }
}

} // namespace

void LogInfo(const std::wstring& message) {
    if (!g_debugLoggingEnabled.load()) {
        return;
    }
    const auto path = std::filesystem::path(PluginDirectory()) / L"aimp_subsonic.log";
    std::ofstream file(path, std::ios::binary | std::ios::app);
    if (!file) {
        return;
    }
    file << WideToUtf8(L"[" + NowText() + L"] " + RedactSensitiveText(message) + L"\r\n");
}

void SetDebugLoggingEnabled(bool enabled) {
    g_debugLoggingEnabled.store(enabled);
}

bool IsDebugLoggingEnabled() {
    return g_debugLoggingEnabled.load();
}

void ShowPluginMessage(const std::wstring& message) {
    LogInfo(message);
    MessageBoxW(nullptr, message.c_str(), L"AIMP Subsonic " AIMP_SUBSONIC_VERSION, MB_OK | MB_ICONINFORMATION);
}

std::wstring FormatLastWindowsError(const std::wstring& prefix) {
    const DWORD error = GetLastError();
    wchar_t* buffer = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        nullptr);

    std::wstring result = prefix + L" (Win32 " + std::to_wstring(error) + L")";
    if (buffer) {
        result += L": ";
        result += buffer;
        LocalFree(buffer);
    }
    return result;
}

std::wstring RedactSensitiveUrl(const std::wstring& url) {
    std::wstring result = url;
    const wchar_t* keys[] = {
        L"u", L"p", L"t", L"s",
        L"username", L"password", L"token", L"salt", L"authorization", L"auth"
    };
    for (const wchar_t* key : keys) {
        RedactQueryParam(result, key);
    }
    return result;
}

std::wstring RedactSensitiveText(const std::wstring& text) {
    std::wstring result = RedactSensitiveUrl(text);
    const wchar_t* keys[] = {
        L"password",
        L"passwordProtected",
        L"token",
        L"salt",
        L"authorization"
    };
    for (const wchar_t* key : keys) {
        RedactKeyValue(result, key);
        RedactKeyValue(result, std::wstring(L"\"") + key + L"\"");
    }
    return result;
}

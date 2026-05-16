#include "Config.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <windows.h>
#include <wincrypt.h>

#include "AimpString.h"
#include "ComBase.h"
#include "Diagnostics.h"
#include "SimpleJson.h"
#include "Url.h"

extern HMODULE g_moduleHandle;

namespace {

const wchar_t* kKeyServerUrl = L"AIMPSubsonic\\ServerUrl";
const wchar_t* kKeyUsername = L"AIMPSubsonic\\Username";
const wchar_t* kKeyPassword = L"AIMPSubsonic\\Password";
const wchar_t* kKeyPasswordProtected = L"AIMPSubsonic\\PasswordProtected";
const wchar_t* kKeyStreamFormat = L"AIMPSubsonic\\StreamFormat";
const wchar_t* kKeyMaxBitRate = L"AIMPSubsonic\\MaxBitRate";
const wchar_t* kKeyLibraryPageSize = L"AIMPSubsonic\\LibraryPageSize";
const wchar_t* kKeyDebugLogging = L"AIMPSubsonic\\DebugLogging";
const wchar_t* kKeyIgnoreTlsCertificateErrors = L"AIMPSubsonic\\IgnoreTlsCertificateErrors";
const wchar_t* kProtectedSecretPrefix = L"dpapi:v1:";

std::optional<std::string> ReadTextFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::filesystem::path ConfigPath() {
    const auto pluginPath = std::filesystem::path(PluginDirectory()) / L"subsonic.local.json";
    if (std::filesystem::exists(pluginPath)) {
        return pluginPath;
    }
    return std::filesystem::current_path() / L"subsonic.local.json";
}

std::wstring BytesToHex(const BYTE* data, DWORD size) {
    static constexpr wchar_t kHex[] = L"0123456789abcdef";
    std::wstring result;
    result.reserve(static_cast<size_t>(size) * 2);
    for (DWORD i = 0; i < size; ++i) {
        result.push_back(kHex[(data[i] >> 4) & 0x0F]);
        result.push_back(kHex[data[i] & 0x0F]);
    }
    return result;
}

int HexValue(wchar_t ch) {
    if (ch >= L'0' && ch <= L'9') return ch - L'0';
    if (ch >= L'a' && ch <= L'f') return ch - L'a' + 10;
    if (ch >= L'A' && ch <= L'F') return ch - L'A' + 10;
    return -1;
}

std::optional<std::vector<BYTE>> HexToBytes(const std::wstring& text) {
    if (text.size() % 2 != 0) {
        return std::nullopt;
    }
    std::vector<BYTE> bytes;
    bytes.reserve(text.size() / 2);
    for (size_t i = 0; i < text.size(); i += 2) {
        const int high = HexValue(text[i]);
        const int low = HexValue(text[i + 1]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        bytes.push_back(static_cast<BYTE>((high << 4) | low));
    }
    return bytes;
}

std::wstring ProtectSecret(const std::wstring& secret) {
    if (secret.empty()) {
        return {};
    }

    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(secret.data()));
    input.cbData = static_cast<DWORD>(secret.size() * sizeof(wchar_t));

    DATA_BLOB output{};
    if (!CryptProtectData(&input, L"AIMP Subsonic password", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output)) {
        LogInfo(FormatLastWindowsError(L"DPAPI password protection failed"));
        return {};
    }

    std::wstring protectedText = kProtectedSecretPrefix;
    protectedText += BytesToHex(output.pbData, output.cbData);
    LocalFree(output.pbData);
    return protectedText;
}

std::optional<std::wstring> UnprotectSecret(const std::wstring& protectedText) {
    if (protectedText.empty()) {
        return std::wstring();
    }
    const std::wstring prefix = kProtectedSecretPrefix;
    if (protectedText.rfind(prefix, 0) != 0) {
        return std::nullopt;
    }

    auto bytes = HexToBytes(protectedText.substr(prefix.size()));
    if (!bytes) {
        LogInfo(L"DPAPI password unprotect skipped: invalid protected payload.");
        return std::nullopt;
    }

    DATA_BLOB input{};
    input.pbData = bytes->data();
    input.cbData = static_cast<DWORD>(bytes->size());
    DATA_BLOB output{};
    LPWSTR description = nullptr;
    if (!CryptUnprotectData(&input, &description, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output)) {
        LogInfo(FormatLastWindowsError(L"DPAPI password unprotect failed"));
        return std::nullopt;
    }
    if (description) {
        LocalFree(description);
    }

    std::optional<std::wstring> result;
    if (output.cbData % sizeof(wchar_t) == 0) {
        result = std::wstring(reinterpret_cast<wchar_t*>(output.pbData), output.cbData / sizeof(wchar_t));
    } else {
        LogInfo(L"DPAPI password unprotect failed: invalid decoded payload size.");
    }
    LocalFree(output.pbData);
    return result;
}

void NormalizeConfig(SubsonicConfig& config) {
    while (!config.serverUrl.empty() && config.serverUrl.back() == L'/') {
        config.serverUrl.pop_back();
    }
    if (config.streamFormat.empty()) {
        config.streamFormat = L"mp3";
    }
    if (config.maxBitRate <= 0) {
        config.maxBitRate = 320;
    }
    if (config.libraryPageSize <= 0) {
        config.libraryPageSize = 500;
    }
    if (config.libraryPageSize > 500) {
        config.libraryPageSize = 500;
    }
}

bool HasConnectionSettings(const SubsonicConfig& config) {
    return !config.serverUrl.empty() && !config.username.empty() && !config.password.empty();
}

IAIMPServiceConfig* QueryConfigService(IAIMPCore* core) {
    IAIMPServiceConfig* config = nullptr;
    if (!core || FAILED(core->QueryInterface(IID_IAIMPServiceConfig, reinterpret_cast<void**>(&config)))) {
        return nullptr;
    }
    return config;
}

std::wstring ConfigGetString(IAIMPCore* core, IAIMPConfig* config, const std::wstring& key) {
    IAIMPString* keyString = MakeAimpString(core, key);
    IAIMPString* valueString = nullptr;
    std::wstring result;
    if (keyString && SUCCEEDED(config->GetValueAsString(keyString, &valueString)) && valueString) {
        result = FromAimpString(valueString);
        valueString->Release();
    }
    SafeRelease(keyString);
    return result;
}

int ConfigGetInt(IAIMPCore* core, IAIMPConfig* config, const std::wstring& key, int fallback) {
    IAIMPString* keyString = MakeAimpString(core, key);
    int value = fallback;
    if (keyString) {
        config->GetValueAsInt32(keyString, &value);
    }
    SafeRelease(keyString);
    return value;
}

bool ConfigGetBool(IAIMPCore* core, IAIMPConfig* config, const std::wstring& key, bool fallback) {
    return ConfigGetInt(core, config, key, fallback ? 1 : 0) != 0;
}

void ConfigSetString(IAIMPCore* core, IAIMPConfig* config, const std::wstring& key, const std::wstring& value) {
    IAIMPString* keyString = MakeAimpString(core, key);
    IAIMPString* valueString = MakeAimpString(core, value);
    if (keyString && valueString) {
        config->SetValueAsString(keyString, valueString);
    }
    SafeRelease(valueString);
    SafeRelease(keyString);
}

void ConfigSetInt(IAIMPCore* core, IAIMPConfig* config, const std::wstring& key, int value) {
    IAIMPString* keyString = MakeAimpString(core, key);
    if (keyString) {
        config->SetValueAsInt32(keyString, value);
    }
    SafeRelease(keyString);
}

} // namespace

std::wstring PluginDirectory() {
    wchar_t path[MAX_PATH]{};
    if (!GetModuleFileNameW(g_moduleHandle, path, MAX_PATH)) {
        return std::filesystem::current_path().wstring();
    }
    return std::filesystem::path(path).parent_path().wstring();
}

SubsonicConfig DefaultSubsonicConfig() {
    SubsonicConfig config;
    config.streamFormat = L"mp3";
    config.maxBitRate = 320;
    config.libraryPageSize = 500;
    config.debugLogging = false;
    config.ignoreTlsCertificateErrors = false;
    return config;
}

std::optional<SubsonicConfig> LoadLocalConfig() {
    const auto text = ReadTextFile(ConfigPath());
    if (!text) {
        return std::nullopt;
    }

    SubsonicConfig config = DefaultSubsonicConfig();
    config.serverUrl = Utf8ToWideString(JsonGetString(*text, "serverUrl").value_or(""));
    config.username = Utf8ToWideString(JsonGetString(*text, "username").value_or(""));
    config.password = Utf8ToWideString(JsonGetString(*text, "password").value_or(""));
    config.streamFormat = Utf8ToWideString(JsonGetString(*text, "streamFormat").value_or("mp3"));
    config.maxBitRate = JsonGetInt(*text, "maxBitRate").value_or(config.maxBitRate);
    config.libraryPageSize = JsonGetInt(*text, "libraryPageSize").value_or(config.libraryPageSize);
    config.debugLogging = JsonGetBool(*text, "debugLogging").value_or(config.debugLogging);
    config.ignoreTlsCertificateErrors = JsonGetBool(*text, "ignoreTlsCertificateErrors").value_or(config.ignoreTlsCertificateErrors);

    NormalizeConfig(config);

    if (!HasConnectionSettings(config)) {
        return std::nullopt;
    }
    return config;
}

std::optional<SubsonicConfig> LoadPluginConfig(IAIMPCore* core) {
    SubsonicConfig config = DefaultSubsonicConfig();
    IAIMPServiceConfig* serviceConfig = QueryConfigService(core);
    if (serviceConfig) {
        config.serverUrl = ConfigGetString(core, serviceConfig, kKeyServerUrl);
        config.username = ConfigGetString(core, serviceConfig, kKeyUsername);
        const std::wstring protectedPassword = ConfigGetString(core, serviceConfig, kKeyPasswordProtected);
        const auto decryptedPassword = UnprotectSecret(protectedPassword);
        config.password = decryptedPassword.value_or(ConfigGetString(core, serviceConfig, kKeyPassword));
        config.streamFormat = ConfigGetString(core, serviceConfig, kKeyStreamFormat);
        config.maxBitRate = ConfigGetInt(core, serviceConfig, kKeyMaxBitRate, config.maxBitRate);
        config.libraryPageSize = ConfigGetInt(core, serviceConfig, kKeyLibraryPageSize, config.libraryPageSize);
        config.debugLogging = ConfigGetBool(core, serviceConfig, kKeyDebugLogging, config.debugLogging);
        config.ignoreTlsCertificateErrors = ConfigGetBool(core, serviceConfig, kKeyIgnoreTlsCertificateErrors, config.ignoreTlsCertificateErrors);
        serviceConfig->Release();
        NormalizeConfig(config);
        if (HasConnectionSettings(config)) {
            LogInfo(L"Config loaded from AIMP settings. Server: " + config.serverUrl);
            return config;
        }
    }

    auto localConfig = LoadLocalConfig();
    if (localConfig) {
        LogInfo(L"Config loaded from subsonic.local.json fallback. Server: " + localConfig->serverUrl);
    }
    return localConfig;
}

bool SavePluginConfig(IAIMPCore* core, const SubsonicConfig& input) {
    IAIMPServiceConfig* serviceConfig = QueryConfigService(core);
    if (!serviceConfig) {
        LogInfo(L"SavePluginConfig failed: IAIMPServiceConfig unavailable.");
        return false;
    }

    SubsonicConfig config = input;
    NormalizeConfig(config);
    ConfigSetString(core, serviceConfig, kKeyServerUrl, config.serverUrl);
    ConfigSetString(core, serviceConfig, kKeyUsername, config.username);
    if (config.password.empty()) {
        ConfigSetString(core, serviceConfig, kKeyPasswordProtected, L"");
        ConfigSetString(core, serviceConfig, kKeyPassword, L"");
    } else {
        const std::wstring protectedPassword = ProtectSecret(config.password);
        if (protectedPassword.empty()) {
            LogInfo(L"SavePluginConfig failed: password could not be protected.");
            serviceConfig->Release();
            return false;
        }
        ConfigSetString(core, serviceConfig, kKeyPasswordProtected, protectedPassword);
        ConfigSetString(core, serviceConfig, kKeyPassword, L"");
    }
    ConfigSetString(core, serviceConfig, kKeyStreamFormat, config.streamFormat);
    ConfigSetInt(core, serviceConfig, kKeyMaxBitRate, config.maxBitRate);
    ConfigSetInt(core, serviceConfig, kKeyLibraryPageSize, config.libraryPageSize);
    ConfigSetInt(core, serviceConfig, kKeyDebugLogging, config.debugLogging ? 1 : 0);
    ConfigSetInt(core, serviceConfig, kKeyIgnoreTlsCertificateErrors, config.ignoreTlsCertificateErrors ? 1 : 0);
    serviceConfig->FlushCache();
    serviceConfig->Release();
    LogInfo(L"Config saved to AIMP settings. Server: " + config.serverUrl +
        L", StreamFormat=" + config.streamFormat +
        L", MaxBitRate=" + std::to_wstring(config.maxBitRate));
    return true;
}

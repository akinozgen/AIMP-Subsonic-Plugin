#pragma once

#include <optional>
#include <string>

#include "apiCore.h"

struct SubsonicConfig {
    std::wstring serverUrl;
    std::wstring username;
    std::wstring password;
    std::wstring streamFormat{L"mp3"};
    int maxBitRate{320};
    int libraryPageSize{500};
    bool debugLogging{false};
    bool ignoreTlsCertificateErrors{false};
};

SubsonicConfig DefaultSubsonicConfig();
std::optional<SubsonicConfig> LoadLocalConfig();
std::optional<SubsonicConfig> LoadPluginConfig(IAIMPCore* core);
bool SavePluginConfig(IAIMPCore* core, const SubsonicConfig& config);
std::wstring PluginDirectory();

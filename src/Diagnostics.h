#pragma once

#include <string>

void LogInfo(const std::wstring& message);
void SetDebugLoggingEnabled(bool enabled);
bool IsDebugLoggingEnabled();
void ShowPluginMessage(const std::wstring& message);
std::wstring FormatLastWindowsError(const std::wstring& prefix);
std::wstring RedactSensitiveText(const std::wstring& text);
std::wstring RedactSensitiveUrl(const std::wstring& url);

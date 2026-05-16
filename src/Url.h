#pragma once

#include <string>

std::string WideToUtf8(const std::wstring& value);
std::wstring Utf8ToWideString(const std::string& value);
std::wstring UrlEncode(const std::wstring& value);
std::wstring UrlDecode(const std::wstring& value);
std::wstring Md5Hex(const std::wstring& value);
std::wstring RandomSalt();
std::wstring SanitizeFilenamePart(const std::wstring& value);

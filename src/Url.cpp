#include "Url.h"

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
#include <vector>
#include <windows.h>
#include <wincrypt.h>

std::string WideToUtf8(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }
    const int length = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), length, nullptr, nullptr);
    return result;
}

std::wstring Utf8ToWideString(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    const int length = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), length);
    return result;
}

std::wstring UrlEncode(const std::wstring& value) {
    const std::string utf8 = WideToUtf8(value);
    std::wostringstream out;
    out << std::uppercase << std::hex;
    for (const unsigned char ch : utf8) {
        const bool unreserved =
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_' || ch == '.' || ch == '~';
        if (unreserved) {
            out << static_cast<wchar_t>(ch);
        } else {
            out << L'%' << std::setw(2) << std::setfill(L'0') << static_cast<int>(ch);
        }
    }
    return out.str();
}

std::wstring UrlDecode(const std::wstring& value) {
    std::string bytes;
    bytes.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        const wchar_t ch = value[i];
        if (ch == L'%' && i + 2 < value.size()) {
            const wchar_t hexChars[3] = { value[i + 1], value[i + 2], 0 };
            wchar_t* end = nullptr;
            const unsigned long decoded = wcstoul(hexChars, &end, 16);
            if (end == hexChars + 2 && decoded <= 0xFF) {
                bytes.push_back(static_cast<char>(decoded));
                i += 2;
                continue;
            }
        }
        if (ch == L'+') {
            bytes.push_back(' ');
        } else if (ch <= 0x7F) {
            bytes.push_back(static_cast<char>(ch));
        } else {
            const std::wstring one(1, ch);
            bytes += WideToUtf8(one);
        }
    }
    return Utf8ToWideString(bytes);
}

std::wstring Md5Hex(const std::wstring& value) {
    const std::string utf8 = WideToUtf8(value);
    HCRYPTPROV provider = 0;
    HCRYPTHASH hash = 0;
    BYTE bytes[16]{};
    DWORD bytesSize = sizeof(bytes);

    if (!CryptAcquireContextW(&provider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return {};
    }
    if (!CryptCreateHash(provider, CALG_MD5, 0, 0, &hash)) {
        CryptReleaseContext(provider, 0);
        return {};
    }
    CryptHashData(hash, reinterpret_cast<const BYTE*>(utf8.data()), static_cast<DWORD>(utf8.size()), 0);
    CryptGetHashParam(hash, HP_HASHVAL, bytes, &bytesSize, 0);
    CryptDestroyHash(hash);
    CryptReleaseContext(provider, 0);

    std::wostringstream out;
    out << std::hex << std::nouppercase << std::setfill(L'0');
    for (BYTE byte : bytes) {
        out << std::setw(2) << static_cast<int>(byte);
    }
    return out.str();
}

std::wstring RandomSalt() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;
    std::wostringstream out;
    out << std::hex << dist(gen) << dist(gen);
    return out.str();
}

std::wstring SanitizeFilenamePart(const std::wstring& value) {
    std::wstring result = value.empty() ? L"unknown" : value;
    for (wchar_t& ch : result) {
        if (ch < 32 || wcschr(L"<>:\"/\\|?*", ch)) {
            ch = L'_';
        }
    }
    if (result.size() > 80) {
        result.resize(80);
    }
    return result;
}

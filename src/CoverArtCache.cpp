#include "CoverArtCache.h"

#include <system_error>
#include <utility>

#include "Config.h"
#include "Diagnostics.h"
#include "Url.h"

namespace {

std::filesystem::path DefaultCoverArtRoot() {
    return std::filesystem::path(PluginDirectory()) / L"cache" / L"covers";
}

bool IsUsableFile(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec) &&
        std::filesystem::is_regular_file(path, ec) &&
        std::filesystem::file_size(path, ec) > 0;
}

} // namespace

CoverArtCache::CoverArtCache(SubsonicClient* client, std::filesystem::path root)
    : client_(client), root_(root.empty() ? DefaultCoverArtRoot() : std::move(root)) {
    std::error_code ec;
    std::filesystem::create_directories(root_, ec);
    LogInfo(L"CoverArtCache initialized. Root=" + root_.wstring() +
        L", CreateDirectoriesError=" + Utf8ToWideString(ec.message()));
}

std::optional<std::filesystem::path> CoverArtCache::GetOrDownload(const std::wstring& coverArtId) {
    if (coverArtId.empty() || !client_) {
        return std::nullopt;
    }

    const auto path = PathForId(coverArtId);
    if (IsUsableFile(path)) {
        return path;
    }

    std::error_code ec;
    std::filesystem::create_directories(root_, ec);
    if (!client_->DownloadCoverArtToFile(coverArtId, path) || !IsUsableFile(path)) {
        std::error_code removeError;
        std::filesystem::remove(path, removeError);
        return std::nullopt;
    }
    return path;
}

std::optional<std::filesystem::path> CoverArtCache::GetCachedPath(const std::wstring& coverArtId) const {
    if (coverArtId.empty()) {
        return std::nullopt;
    }
    const auto path = PathForId(coverArtId);
    return IsUsableFile(path) ? std::optional<std::filesystem::path>(path) : std::nullopt;
}

int CoverArtCache::Clear() {
    int removed = 0;
    std::error_code ec;
    if (!std::filesystem::exists(root_, ec)) {
        return 0;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root_, ec)) {
        if (ec) {
            break;
        }
        std::error_code fileError;
        if (entry.is_regular_file(fileError) && std::filesystem::remove(entry.path(), fileError)) {
            ++removed;
        }
    }
    LogInfo(L"CoverArtCache cleared. RemovedFiles=" + std::to_wstring(removed));
    return removed;
}

std::pair<int, unsigned long long> CoverArtCache::Stats() const {
    int files = 0;
    unsigned long long bytes = 0;
    std::error_code ec;
    if (!std::filesystem::exists(root_, ec)) {
        return { files, bytes };
    }
    for (const auto& entry : std::filesystem::directory_iterator(root_, ec)) {
        if (ec) {
            break;
        }
        std::error_code fileError;
        if (entry.is_regular_file(fileError)) {
            const auto size = entry.file_size(fileError);
            if (fileError) {
                continue;
            }
            ++files;
            bytes += static_cast<unsigned long long>(size);
        }
    }
    return { files, bytes };
}

std::filesystem::path CoverArtCache::PathForId(const std::wstring& coverArtId) const {
    std::wstring name = Md5Hex(coverArtId);
    if (name.empty()) {
        name = SanitizeFilenamePart(coverArtId);
    }
    return root_ / (name + L".img");
}

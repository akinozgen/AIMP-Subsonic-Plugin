#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include "SubsonicClient.h"

class CoverArtCache {
public:
    explicit CoverArtCache(SubsonicClient* client, std::filesystem::path root = {});

    std::optional<std::filesystem::path> GetOrDownload(const std::wstring& coverArtId);
    std::optional<std::filesystem::path> GetCachedPath(const std::wstring& coverArtId) const;
    int Clear();
    std::pair<int, unsigned long long> Stats() const;
    std::filesystem::path Root() const { return root_; }

private:
    std::filesystem::path PathForId(const std::wstring& coverArtId) const;

    SubsonicClient* client_;
    std::filesystem::path root_;
};

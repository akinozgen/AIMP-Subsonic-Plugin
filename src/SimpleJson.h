#pragma once

#include <optional>
#include <string>
#include <vector>

struct TrackInfo {
    std::wstring id;
    std::wstring title;
    std::wstring artist;
    std::wstring album;
    std::wstring albumArtist;
    std::wstring artistId;
    std::wstring albumId;
    std::wstring coverArt;
    std::wstring genre;
    std::wstring playlistId;
    std::wstring suffix;
    int durationSeconds{0};
    int year{0};
    int trackNumber{0};
    int discNumber{0};
    int rating{0};
    long long size{0};
    bool starred{false};
};

struct PlaylistInfo {
    std::wstring id;
    std::wstring name;
    std::wstring owner;
    std::vector<std::wstring> songIds;
    int songCount{0};
    int durationSeconds{0};
    bool isPublic{false};
};

struct ArtistInfo {
    std::wstring id;
    std::wstring name;
    std::wstring coverArt;
    std::vector<std::wstring> albumIds;
    int albumCount{0};
};

struct AlbumInfo {
    std::wstring id;
    std::wstring name;
    std::wstring artist;
    std::wstring artistId;
    std::wstring coverArt;
    std::wstring genre;
    std::vector<std::wstring> songIds;
    int songCount{0};
    int durationSeconds{0};
    int year{0};
};

std::optional<std::string> JsonGetString(const std::string& objectText, const std::string& key);
std::optional<int> JsonGetInt(const std::string& objectText, const std::string& key);
std::optional<long long> JsonGetInt64(const std::string& objectText, const std::string& key);
std::optional<bool> JsonGetBool(const std::string& objectText, const std::string& key);
std::vector<std::string> JsonGetStringArray(const std::string& objectText, const std::string& key);
std::vector<TrackInfo> ParseRandomSongs(const std::string& json);
std::vector<TrackInfo> ParseTrackArray(const std::string& json, const std::string& key);
std::vector<PlaylistInfo> ParsePlaylists(const std::string& json);
std::vector<PlaylistInfo> ParsePlaylistsForKey(const std::string& json, const std::string& key);
std::vector<ArtistInfo> ParseArtists(const std::string& json);
std::vector<ArtistInfo> ParseArtistsForKey(const std::string& json, const std::string& key);
std::vector<AlbumInfo> ParseAlbums(const std::string& json, const std::string& key);

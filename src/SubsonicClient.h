#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Config.h"
#include "SimpleJson.h"

class SubsonicClient {
public:
    explicit SubsonicClient(SubsonicConfig config);

    bool Ping() const;
    std::vector<TrackInfo> GetStarredSongs() const;
    std::vector<ArtistInfo> GetArtists() const;
    std::vector<AlbumInfo> GetArtistAlbums(const std::wstring& artistId) const;
    std::vector<AlbumInfo> GetAlbumList2(const std::wstring& type, int size, int offset = 0) const;
    std::vector<TrackInfo> GetAlbumTracks(const std::wstring& albumId) const;
    std::optional<TrackInfo> GetSong(const std::wstring& id) const;
    std::vector<PlaylistInfo> GetPlaylists() const;
    std::vector<TrackInfo> GetPlaylistTracks(const std::wstring& playlistId) const;
    std::vector<TrackInfo> SearchSongs(const std::wstring& query, int count, int offset = 0) const;
    bool Scrobble(const std::wstring& songId, bool submission) const;
    bool DownloadCoverArtToFile(const std::wstring& coverArtId, const std::filesystem::path& destination) const;
    std::wstring BuildStreamUrl(const std::wstring& id) const;
    std::wstring BuildCoverArtUrl(const std::wstring& coverArtId) const;
    std::wstring MetadataCacheKey() const;
    int LibraryPageSize() const;

private:
    std::wstring ApiUrl(const std::wstring& method, const std::wstring& extraQuery) const;
    std::wstring AuthQuery() const;

    SubsonicConfig config_;
};

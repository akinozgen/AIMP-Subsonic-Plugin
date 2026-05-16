#pragma once

#include <string>

enum class SubsonicLibraryNodeKind {
    Unknown,
    Root,
    PlaylistsRoot,
    ArtistsRoot,
    AlbumsRoot,
    FavoritesRoot,
    TracksRoot,
    Playlist,
    Artist,
    Album,
};

struct SubsonicLibraryNodeRef {
    SubsonicLibraryNodeKind kind{SubsonicLibraryNodeKind::Unknown};
    std::wstring path;
    std::wstring id;
};

const wchar_t* SubsonicLibraryNodeKindName(SubsonicLibraryNodeKind kind);
std::wstring FormatSubsonicLibraryNodeRef(const SubsonicLibraryNodeRef& node);

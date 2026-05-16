#include "SubsonicLibraryNodes.h"

const wchar_t* SubsonicLibraryNodeKindName(SubsonicLibraryNodeKind kind) {
    switch (kind) {
    case SubsonicLibraryNodeKind::Root:
        return L"Root";
    case SubsonicLibraryNodeKind::PlaylistsRoot:
        return L"PlaylistsRoot";
    case SubsonicLibraryNodeKind::ArtistsRoot:
        return L"ArtistsRoot";
    case SubsonicLibraryNodeKind::AlbumsRoot:
        return L"AlbumsRoot";
    case SubsonicLibraryNodeKind::FavoritesRoot:
        return L"FavoritesRoot";
    case SubsonicLibraryNodeKind::TracksRoot:
        return L"TracksRoot";
    case SubsonicLibraryNodeKind::Playlist:
        return L"Playlist";
    case SubsonicLibraryNodeKind::Artist:
        return L"Artist";
    case SubsonicLibraryNodeKind::Album:
        return L"Album";
    case SubsonicLibraryNodeKind::Unknown:
    default:
        return L"Unknown";
    }
}

std::wstring FormatSubsonicLibraryNodeRef(const SubsonicLibraryNodeRef& node) {
    std::wstring result = L"NodeRef{kind=";
    result += SubsonicLibraryNodeKindName(node.kind);
    result += L", path='";
    result += node.path;
    result += L"', id='";
    result += node.id.empty() ? L"<unresolved>" : node.id;
    result += L"'}";
    return result;
}

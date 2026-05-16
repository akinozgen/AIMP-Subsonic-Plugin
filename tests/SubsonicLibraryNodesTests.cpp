#include "SubsonicLibraryNodes.h"

#include <iostream>

namespace {

bool ExpectEqual(const std::wstring& actual, const std::wstring& expected, const char* name) {
    if (actual == expected) {
        return true;
    }
    std::wcerr << L"FAILED " << name << L": expected '" << expected << L"', got '" << actual << L"'\n";
    return false;
}

} // namespace

int wmain() {
    bool ok = true;
    ok = ExpectEqual(SubsonicLibraryNodeKindName(SubsonicLibraryNodeKind::TracksRoot), L"TracksRoot", "TracksRoot name") && ok;
    ok = ExpectEqual(SubsonicLibraryNodeKindName(SubsonicLibraryNodeKind::Playlist), L"Playlist", "Playlist name") && ok;

    SubsonicLibraryNodeRef unresolved;
    unresolved.kind = SubsonicLibraryNodeKind::Album;
    unresolved.path = L"Albums\\Example\\";
    ok = ExpectEqual(
        FormatSubsonicLibraryNodeRef(unresolved),
        L"NodeRef{kind=Album, path='Albums\\Example\\', id='<unresolved>'}",
        "Unresolved node format") && ok;

    SubsonicLibraryNodeRef resolved;
    resolved.kind = SubsonicLibraryNodeKind::Artist;
    resolved.path = L"Artists\\Example\\";
    resolved.id = L"artist-1";
    ok = ExpectEqual(
        FormatSubsonicLibraryNodeRef(resolved),
        L"NodeRef{kind=Artist, path='Artists\\Example\\', id='artist-1'}",
        "Resolved node format") && ok;

    return ok ? 0 : 1;
}

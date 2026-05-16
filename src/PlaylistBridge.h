#pragma once

#include <string>

#include "apiFileManager.h"
#include "apiPlaylists.h"

#include "SimpleJson.h"

namespace PlaylistBridge {

std::wstring TrackDisplayText(const TrackInfo& track);
std::wstring ExtractSubsonicSongId(const std::wstring& uri);
std::wstring ExtractSubsonicSongId(IAIMPFileInfo* info);
std::wstring GetFileInfoString(IAIMPFileInfo* info, int propId);
std::wstring GetPlaylistItemString(IAIMPPlaylistItem* item, int propId);
TrackInfo BuildTrackFromFileInfo(const std::wstring& songId, IAIMPFileInfo* info);

} // namespace PlaylistBridge

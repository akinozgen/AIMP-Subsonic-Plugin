#include "SubsonicFileInfoProvider.h"

#include "Diagnostics.h"
#include "PlaylistBridge.h"

namespace {

HRESULT SetStringProp(IAIMPCore* core, IAIMPFileInfo* info, int propId, const std::wstring& value) {
    if (value.empty()) {
        return S_OK;
    }
    IAIMPString* str = MakeAimpString(core, value);
    if (!str) {
        return E_FAIL;
    }
    const HRESULT hr = info->SetValueAsObject(propId, str);
    str->Release();
    return hr;
}

} // namespace

SubsonicFileInfoProvider::SubsonicFileInfoProvider(IAIMPCore* core, SubsonicRepository* repository)
    : core_(core), repository_(repository) {
}

HRESULT WINAPI SubsonicFileInfoProvider::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) {
        return E_POINTER;
    }
    *ppvObject = nullptr;
    if (riid == IID_IUnknown || riid == IID_IAIMPExtensionFileInfoProvider) {
        *ppvObject = static_cast<IAIMPExtensionFileInfoProvider*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

HRESULT WINAPI SubsonicFileInfoProvider::GetFileInfo(IAIMPString* FileURI, IAIMPFileInfo* Info) {
    if (!FileURI || !Info) {
        return E_POINTER;
    }

    const std::wstring uri = FromAimpString(FileURI);
    const std::wstring songId = PlaylistBridge::ExtractSubsonicSongId(uri);
    if (songId.empty()) {
        return E_FAIL;
    }

    const auto track = repository_ ? repository_->GetSong(songId) : std::nullopt;
    if (!track) {
        LogInfo(L"Subsonic file info provider: metadata not found. Id=" + songId);
        return E_FAIL;
    }

    return FillFileInfo(uri, *track, Info);
}

void SubsonicFileInfoProvider::SetRepository(SubsonicRepository* repository) {
    repository_ = repository;
    LogInfo(L"Subsonic file info provider repository updated.");
}

HRESULT SubsonicFileInfoProvider::FillFileInfo(const std::wstring& uri, const TrackInfo& track, IAIMPFileInfo* info) const {
    SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_FILENAME, uri);
    SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_URL, uri);
    SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_TITLE, track.title);
    SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_ARTIST, track.artist);
    SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_ALBUM, track.album);
    SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_ALBUMARTIST, track.albumArtist);
    SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_GENRE, track.genre);
    if (track.year > 0) {
        SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_DATE, std::to_wstring(track.year));
    }
    if (track.trackNumber > 0) {
        SetStringProp(core_.get(), info, AIMP_FILEINFO_PROPID_TRACKNUMBER, std::to_wstring(track.trackNumber));
    }
    if (track.rating > 0) {
        info->SetValueAsInt32(AIMP_FILEINFO_PROPID_RATING, track.rating);
    }
    if (track.durationSeconds > 0) {
        info->SetValueAsFloat(AIMP_FILEINFO_PROPID_DURATION, static_cast<double>(track.durationSeconds));
    }
    if (track.size > 0) {
        info->SetValueAsInt64(AIMP_FILEINFO_PROPID_FILESIZE, track.size);
    }
    return S_OK;
}

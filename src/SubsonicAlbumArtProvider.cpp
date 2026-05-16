#include "SubsonicAlbumArtProvider.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>

#include "Diagnostics.h"
#include "Url.h"

namespace {

std::wstring GetFileInfoString(IAIMPFileInfo* info, int propId) {
    if (!info) {
        return {};
    }
    IAIMPString* value = nullptr;
    std::wstring result;
    if (SUCCEEDED(info->GetValueAsObject(propId, IID_IAIMPString, reinterpret_cast<void**>(&value))) && value) {
        result = FromAimpString(value);
        value->Release();
    }
    return result;
}

bool StartsWithIgnoreCase(const std::wstring& value, const std::wstring& prefix) {
    return value.size() >= prefix.size() && _wcsnicmp(value.c_str(), prefix.c_str(), prefix.size()) == 0;
}

std::wstring ExtractQueryParam(const std::wstring& uri, const std::wstring& key) {
    const size_t question = uri.find(L'?');
    if (question == std::wstring::npos) {
        return {};
    }
    const std::wstring needle = key + L"=";
    size_t pos = question + 1;
    while (pos < uri.size()) {
        const size_t next = uri.find(L'&', pos);
        const size_t length = next == std::wstring::npos ? std::wstring::npos : next - pos;
        const std::wstring part = uri.substr(pos, length);
        if (part.rfind(needle, 0) == 0) {
            return UrlDecode(part.substr(needle.size()));
        }
        if (next == std::wstring::npos) {
            break;
        }
        pos = next + 1;
    }
    return {};
}

std::wstring ExtractSubsonicSongId(const std::wstring& uri) {
    const std::wstring id = ExtractQueryParam(uri, L"id");
    if (!id.empty() && uri.find(L"/rest/stream") != std::wstring::npos) {
        return id;
    }

    const std::wstring virtualPrefix = L"subsonic:\\\\song\\";
    if (StartsWithIgnoreCase(uri, virtualPrefix)) {
        std::wstring tail = uri.substr(virtualPrefix.size());
        const size_t slash = tail.find_first_of(L"\\/");
        if (slash != std::wstring::npos) {
            tail.resize(slash);
        }
        return tail;
    }
    return {};
}

} // namespace

SubsonicAlbumArtProvider::SubsonicAlbumArtProvider(IAIMPCore* core, SubsonicRepository* repository, CoverArtCache* coverCache)
    : core_(core), repository_(repository), coverCache_(coverCache) {
}

HRESULT WINAPI SubsonicAlbumArtProvider::QueryInterface(REFIID riid, void** ppvObject) {
    if (!ppvObject) {
        return E_POINTER;
    }
    *ppvObject = nullptr;
    if (riid == IID_IUnknown || riid == IID_IAIMPExtensionAlbumArtProvider3) {
        *ppvObject = static_cast<IAIMPExtensionAlbumArtProvider3*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

HRESULT WINAPI SubsonicAlbumArtProvider::Get(IAIMPFileInfo* FileInfo, IAIMPAlbumArtRequest* Request, IAIMPImageContainer** Image) {
    if (!Image) {
        return E_POINTER;
    }
    *Image = nullptr;
    const auto coverArtId = ResolveCoverArtId(FileInfo);
    if (!coverArtId || coverArtId->empty() || !coverCache_) {
        return E_FAIL;
    }

    IAIMPString* cacheKey = MakeAimpString(core_.get(), L"aimp-subsonic-cover:" + *coverArtId);
    if (Request && cacheKey && SUCCEEDED(Request->CacheGet(cacheKey, Image)) && *Image) {
        cacheKey->Release();
        return S_OK;
    }

    const auto path = coverCache_->GetOrDownload(*coverArtId);
    if (!path) {
        SafeRelease(cacheKey);
        LogInfo(L"Album art provider failed: cover was not downloaded. CoverArtId=" + *coverArtId);
        return E_FAIL;
    }

    const HRESULT hr = LoadImageContainer(*path, Image);
    if (SUCCEEDED(hr) && Request && cacheKey && *Image) {
        Request->CachePut(cacheKey, Image);
    }
    SafeRelease(cacheKey);
    return hr;
}

LongWord WINAPI SubsonicAlbumArtProvider::GetCategory() {
    return AIMP_ALBUMART_PROVIDER_CATEGORY_INTERNET;
}

void SubsonicAlbumArtProvider::SetServices(SubsonicRepository* repository, CoverArtCache* coverCache) {
    repository_ = repository;
    coverCache_ = coverCache;
    LogInfo(L"SubsonicAlbumArtProvider services updated.");
}

std::wstring SubsonicAlbumArtProvider::ExtractSongId(IAIMPFileInfo* info) const {
    std::wstring songId = ExtractSubsonicSongId(GetFileInfoString(info, AIMP_FILEINFO_PROPID_FILENAME));
    if (songId.empty()) {
        songId = ExtractSubsonicSongId(GetFileInfoString(info, AIMP_FILEINFO_PROPID_URL));
    }
    return songId;
}

std::optional<std::wstring> SubsonicAlbumArtProvider::ResolveCoverArtId(IAIMPFileInfo* info) const {
    const std::wstring songId = ExtractSongId(info);
    const auto track = repository_ && !songId.empty() ? repository_->FindTrackMetadata(songId) : std::nullopt;
    if (!track) {
        return std::nullopt;
    }
    if (!track->coverArt.empty()) {
        return track->coverArt;
    }
    if (!track->albumId.empty()) {
        return track->albumId;
    }
    return std::nullopt;
}

HRESULT SubsonicAlbumArtProvider::LoadImageContainer(const std::filesystem::path& path, IAIMPImageContainer** image) const {
    if (!image) {
        return E_POINTER;
    }
    *image = nullptr;

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return E_FAIL;
    }
    std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty()) {
        return E_FAIL;
    }

    IAIMPImageContainer* container = nullptr;
    HRESULT hr = core_.get()->CreateObject(IID_IAIMPImageContainer, reinterpret_cast<void**>(&container));
    if (FAILED(hr) || !container) {
        return FAILED(hr) ? hr : E_FAIL;
    }
    hr = container->SetDataSize(static_cast<LongWord>(bytes.size()));
    if (SUCCEEDED(hr)) {
        byte* data = container->GetData();
        if (data) {
            std::copy(bytes.begin(), bytes.end(), data);
            *image = container;
            return S_OK;
        }
        hr = E_FAIL;
    }
    container->Release();
    return hr;
}

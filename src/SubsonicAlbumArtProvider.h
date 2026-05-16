#pragma once

#include <optional>

#include "apiAlbumArt.h"
#include "apiCore.h"

#include "AimpString.h"
#include "ComBase.h"
#include "CoverArtCache.h"
#include "SubsonicRepository.h"

class SubsonicAlbumArtProvider final : public IAIMPExtensionAlbumArtProvider3, public ComBase {
public:
    SubsonicAlbumArtProvider(IAIMPCore* core, SubsonicRepository* repository, CoverArtCache* coverCache);

    HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG WINAPI AddRef() override { return AddRefImpl(); }
    ULONG WINAPI Release() override { return ReleaseImpl(); }

    HRESULT WINAPI Get(IAIMPFileInfo* FileInfo, IAIMPAlbumArtRequest* Request, IAIMPImageContainer** Image) override;
    LongWord WINAPI GetCategory() override;

    void SetServices(SubsonicRepository* repository, CoverArtCache* coverCache);

private:
    std::wstring ExtractSongId(IAIMPFileInfo* info) const;
    std::optional<std::wstring> ResolveCoverArtId(IAIMPFileInfo* info) const;
    HRESULT LoadImageContainer(const std::filesystem::path& path, IAIMPImageContainer** image) const;

    AimpCoreRef core_;
    SubsonicRepository* repository_;
    CoverArtCache* coverCache_;
};

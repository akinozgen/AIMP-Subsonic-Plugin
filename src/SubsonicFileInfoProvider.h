#pragma once

#include "apiFileManager.h"

#include "AimpString.h"
#include "ComBase.h"
#include "SubsonicRepository.h"

class SubsonicFileInfoProvider final : public IAIMPExtensionFileInfoProvider, public ComBase {
public:
    SubsonicFileInfoProvider(IAIMPCore* core, SubsonicRepository* repository);

    HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG WINAPI AddRef() override { return AddRefImpl(); }
    ULONG WINAPI Release() override { return ReleaseImpl(); }

    HRESULT WINAPI GetFileInfo(IAIMPString* FileURI, IAIMPFileInfo* Info) override;

    void SetRepository(SubsonicRepository* repository);

private:
    HRESULT FillFileInfo(const std::wstring& uri, const TrackInfo& track, IAIMPFileInfo* info) const;

    AimpCoreRef core_;
    SubsonicRepository* repository_{nullptr};
};

#pragma once

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "apiCore.h"
#include "apiMusicLibrary.h"

#include "AimpString.h"
#include "ComBase.h"
#include "SimpleJson.h"
#include "SubsonicLibraryNodes.h"
#include "SubsonicRepository.h"

class SubsonicMusicLibraryStorage final :
    public IAIMPMLExtensionDataStorage,
    public IAIMPMLDataProvider,
    public IAIMPMLGroupingTreeDataProvider2,
    public ComBase {
public:
    struct LibraryRow;

    SubsonicMusicLibraryStorage(IAIMPCore* core, SubsonicRepository* repository);
    ~SubsonicMusicLibraryStorage() override;
    void SetServices(SubsonicRepository* repository);
    void NotifyChanged();

    HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG WINAPI AddRef() override { return AddRefImpl(); }
    ULONG WINAPI Release() override { return ReleaseImpl(); }

    void WINAPI BeginUpdate() override {}
    void WINAPI EndUpdate() override {}
    HRESULT WINAPI Reset() override { return S_OK; }
    HRESULT WINAPI GetValueAsFloat(int PropertyID, double* Value) override;
    HRESULT WINAPI GetValueAsInt32(int PropertyID, int* Value) override;
    HRESULT WINAPI GetValueAsInt64(int PropertyID, INT64* Value) override;
    HRESULT WINAPI GetValueAsObject(int PropertyID, REFIID IID, void** Value) override;
    HRESULT WINAPI SetValueAsFloat(int PropertyID, const double Value) override;
    HRESULT WINAPI SetValueAsInt32(int PropertyID, int Value) override;
    HRESULT WINAPI SetValueAsInt64(int PropertyID, const INT64 Value) override;
    HRESULT WINAPI SetValueAsObject(int PropertyID, IUnknown* Value) override;

    void WINAPI Finalize() override;
    void WINAPI Initialize(IAIMPMLDataStorageManager* Manager) override;
    HRESULT WINAPI ConfigLoad(IAIMPConfig* Config, IAIMPString* Section) override;
    HRESULT WINAPI ConfigSave(IAIMPConfig* Config, IAIMPString* Section) override;
    HRESULT WINAPI GetFields(int Schema, IAIMPObjectList** List) override;
    HRESULT WINAPI GetGroupingPresets(int Schema, IAIMPMLGroupingPresets* Presets) override;
    void WINAPI FlushCache(int Reserved) override;

    HRESULT WINAPI GetData(IAIMPObjectList* Fields, IAIMPMLDataFilter* Filter, IUnknown** Data) override;
    HRESULT WINAPI AppendFilter(IAIMPMLDataFilterGroup* Filter, IAIMPMLGroupingTreeSelection* Selection) override;
    LongWord WINAPI GetCapabilities() override;
    HRESULT WINAPI GetData(IAIMPMLGroupingTreeSelection* Selection, IAIMPMLGroupingTreeDataProviderSelection** Data) override;
    HRESULT WINAPI GetFieldForAlphabeticIndex(IAIMPString** FieldName) override;
    TChar WINAPI GetPathSeparator() override;

private:
    IAIMPMLDataField* CreateField(const std::wstring& name, int type, int flags);
    HRESULT AddFieldName(IAIMPObjectList* list, const std::wstring& name);
    std::vector<std::wstring> ReadFieldNames(IAIMPObjectList* fields) const;
    void CollectNodeFilters(IAIMPMLDataFilterGroup* filter, std::vector<std::wstring>& nodes) const;
    std::wstring ExtractNodeFilter(IAIMPMLDataFilterGroup* filter) const;
    std::wstring ExtractTreeSelectionNode(IAIMPMLGroupingTreeSelection* selection) const;
    std::vector<LibraryRow> BuildGroupingRows(const std::wstring& node);
    std::vector<LibraryRow> BuildTableRows(const std::wstring& node, const std::wstring& searchText);
    std::vector<TrackInfo> LoadTracksFromAlbumScan(int maxTracks);
    void RememberTracks(const std::vector<TrackInfo>& tracks);
    std::vector<PlaylistInfo> LoadPlaylists();
    std::vector<ArtistInfo> LoadArtists();
    std::vector<AlbumInfo> LoadArtistAlbums(const std::wstring& artistId);
    std::vector<AlbumInfo> LoadAlbums();
    std::wstring ResolveNodeId(const std::map<std::wstring, std::wstring>& index, const std::wstring& node) const;
    SubsonicLibraryNodeRef ResolveNodeForDiagnostics(const std::wstring& node) const;

    AimpCoreRef core_;
    SubsonicRepository* repository_;
    IAIMPMLDataStorageManager* manager_{nullptr};
    mutable std::mutex indexMutex_;
    std::map<std::wstring, TrackInfo> trackIndex_;
    std::map<std::wstring, std::wstring> artistNodeIndex_;
    std::map<std::wstring, std::wstring> albumNodeIndex_;
    std::map<std::wstring, std::wstring> playlistNodeIndex_;
    std::vector<PlaylistInfo> playlistIndex_;
    std::vector<ArtistInfo> artistIndex_;
    std::vector<AlbumInfo> albumIndex_;
};

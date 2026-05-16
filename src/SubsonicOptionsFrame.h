#pragma once

#include <string>
#include <vector>
#include <windows.h>

#include "apiOptions.h"
#include "apiGUI.h"

#include "AimpString.h"
#include "ComBase.h"
#include "Config.h"

class AimpSubsonicPlugin;
class UiChangeHandler;

class SubsonicOptionsFrame final : public IAIMPOptionsDialogFrame, public ComBase {
public:
    SubsonicOptionsFrame(IAIMPCore* core, AimpSubsonicPlugin* plugin);
    ~SubsonicOptionsFrame() override;

    HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG WINAPI AddRef() override { return AddRefImpl(); }
    ULONG WINAPI Release() override { return ReleaseImpl(); }

    HRESULT WINAPI GetName(IAIMPString** S) override;
    HWND WINAPI CreateFrame(HWND ParentWnd) override;
    void WINAPI DestroyFrame() override;
    void WINAPI Notification(int ID) override;
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    friend class UiChangeHandler;
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    void CreateControls();
    bool CreateAimpControls(HWND parentWnd);
    void LayoutControls();
    void LayoutAimpControls();
    void LoadIntoControls(const SubsonicConfig& config);
    SubsonicConfig ReadFromControls() const;
    void SaveFromControls();
    void TestConnection();
    void RedrawFrame();
    void MarkModified();
    void SetStatus(const std::wstring& value);
    void UpdateTheme();
    void DestroyThemeBrushes();

    HWND CreateLabel(const std::wstring& caption);
    HWND CreateHint(const std::wstring& caption);
    HWND CreateGroupBox(const std::wstring& caption);
    HWND CreateEdit(int id, DWORD style = 0);
    HWND CreateButton(int id, const std::wstring& caption);
    std::wstring GetControlText(HWND hwnd) const;
    void SetControlText(HWND hwnd, const std::wstring& text);

    AimpCoreRef core_;
    AimpSubsonicPlugin* plugin_;
    HWND host_{nullptr};
    IAIMPUIForm* uiForm_{nullptr};
    IAIMPServiceUI* uiService_{nullptr};
    IAIMPUIGroupBox* uiConnectionGroup_{nullptr};
    IAIMPUIGroupBox* uiPlaybackGroup_{nullptr};
    IAIMPUIGroupBox* uiLibraryGroup_{nullptr};
    IAIMPUIGroupBox* uiDiagnosticsGroup_{nullptr};
    IAIMPUIEdit* uiServerUrlEdit_{nullptr};
    IAIMPUIEdit* uiUsernameEdit_{nullptr};
    IAIMPUIEdit* uiPasswordEdit_{nullptr};
    IAIMPUIEdit* uiStreamFormatEdit_{nullptr};
    IAIMPUIEdit* uiMaxBitRateEdit_{nullptr};
    IAIMPUIEdit* uiLibraryPageSizeEdit_{nullptr};
    IAIMPUICheckBox* uiShowPasswordCheck_{nullptr};
    IAIMPUICheckBox* uiIgnoreTlsCertificateErrorsCheck_{nullptr};
    IAIMPUICheckBox* uiDebugLoggingCheck_{nullptr};
    IAIMPUIButton* uiTestButton_{nullptr};
    IAIMPUILabel* uiStatusLabel_{nullptr};
    std::vector<IAIMPUILabel*> uiLabels_;
    std::vector<IAIMPUILabel*> uiHints_;
    std::vector<IUnknown*> uiEventHandlers_;
    HWND serverUrlEdit_{nullptr};
    HWND usernameEdit_{nullptr};
    HWND passwordEdit_{nullptr};
    HWND showPasswordCheck_{nullptr};
    HWND streamFormatEdit_{nullptr};
    HWND maxBitRateEdit_{nullptr};
    HWND libraryPageSizeEdit_{nullptr};
    HWND ignoreTlsCertificateErrorsCheck_{nullptr};
    HWND debugLoggingCheck_{nullptr};
    HWND testButton_{nullptr};
    HWND statusLabel_{nullptr};
    HWND connectionGroup_{nullptr};
    HWND playbackGroup_{nullptr};
    HWND libraryGroup_{nullptr};
    HWND diagnosticsGroup_{nullptr};
    std::vector<HWND> labels_;
    std::vector<HWND> hints_;
    COLORREF backgroundColor_{RGB(255, 255, 255)};
    COLORREF textColor_{RGB(0, 0, 0)};
    COLORREF hintTextColor_{RGB(80, 80, 80)};
    COLORREF editBackgroundColor_{RGB(255, 255, 255)};
    COLORREF editTextColor_{RGB(0, 0, 0)};
    HBRUSH backgroundBrush_{nullptr};
    HBRUSH editBackgroundBrush_{nullptr};
    bool darkTheme_{false};
    bool loading_{false};
};

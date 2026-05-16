#include <windows.h>

#include "AimpPlugin.h"

HMODULE g_moduleHandle = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_moduleHandle = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}

extern "C" __declspec(dllexport) HRESULT WINAPI AIMPPluginGetHeader(IAIMPPlugin** Header) {
    if (!Header) {
        return E_POINTER;
    }
    *Header = new AimpSubsonicPlugin();
    return *Header ? S_OK : E_OUTOFMEMORY;
}

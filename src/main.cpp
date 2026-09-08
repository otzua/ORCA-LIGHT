#include <windows.h>
#include "app.h"
#include "utils/logger.h"

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    // CRITICAL: Initialize logger first to catch OS-level load issues if possible
    orca_light::utils::Logger::init();
    orca_light::utils::Logger::log("Orca-Light wWinMain started.");

    // Dynamically resolve Per-Monitor V2 DPI awareness if supported
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* SetDpiContextFn)(DPI_AWARENESS_CONTEXT);
        SetDpiContextFn fn = reinterpret_cast<SetDpiContextFn>(GetProcAddress(hUser32, "SetProcessDpiAwarenessContext"));
        if (fn) {
            fn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
    }

    // Initialize COM for Windows Shell and WIC
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to initialize COM library.", L"Orca-Light Initialization Error", MB_ICONERROR);
        return 1;
    }

    int exit_code = 0;
    {
        orca_light::Application app;
        if (app.initialize(hInstance)) {
            exit_code = app.run();
        }
    }

    CoUninitialize();
    return exit_code;
}

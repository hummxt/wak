#include "core/Application.h"
#include "utils/Logger.h"

#include <Windows.h>
#include <shellapi.h>
#include <objbase.h>

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

namespace {

void EnableDpiAwareness() {
    using SetContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        auto setContext = reinterpret_cast<SetContextFn>(GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
        if (setContext && setContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            return;
        }
    }

    using SetAwarenessFn = HRESULT(WINAPI*)(int);
    HMODULE shcore = LoadLibraryW(L"Shcore.dll");
    if (shcore) {
        auto setAwareness = reinterpret_cast<SetAwarenessFn>(GetProcAddress(shcore, "SetProcessDpiAwareness"));
        if (setAwareness && SUCCEEDED(setAwareness(2))) {
            return;
        }
    }

    SetProcessDPIAware();
}

}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int nCmdShow) {
    EnableDpiAwareness();
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool background = false;
    bool autoApply = false;
    if (argv) {
        for (int i = 0; i < argc; ++i) {
            if (wcscmp(argv[i], L"--background") == 0) {
                background = true;
            }
            if (wcscmp(argv[i], L"--apply-desktop") == 0) {
                autoApply = true;
                background = true;
            }
        }
        LocalFree(argv);
    }

    ge::Application app;
    if (!app.Initialize(instance, background ? SW_HIDE : nCmdShow, background)) {
        ge::LogError("Application failed to initialize");
        return 1;
    }

    if (autoApply) {
        app.ApplyDesktopForTesting();
    }

    const int code = app.Run();
    app.Shutdown();
    CoUninitialize();
    return code;
}

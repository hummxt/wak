#include "core/Application.h"
#include "utils/Logger.h"

#include <Windows.h>
#include <shellapi.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int nCmdShow) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool autoApply = false;
    if (argv) {
        for (int i = 0; i < argc; ++i) {
            if (wcscmp(argv[i], L"--apply-desktop") == 0) {
                autoApply = true;
            }
        }
        LocalFree(argv);
    }

    ge::Application app;
    if (!app.Initialize(instance, nCmdShow)) {
        ge::LogError("Application failed to initialize");
        return 1;
    }

    if (autoApply) {
        app.ApplyDesktopForTesting();
    }

    const int code = app.Run();
    app.Shutdown();
    return code;
}

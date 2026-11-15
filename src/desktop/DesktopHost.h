#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

namespace ge {

class DesktopHost {
public:
    bool Attach();
    void Detach();
    bool EmbedWindow(HWND hwnd);
    void ReleaseWindow(HWND hwnd);
    HWND GetHostWindow() const;
    HWND GetDefView() const;
    HWND GetProgman() const;
    HWND GetWorkerW() const;
    bool IsRaisedDesktop() const;
    bool IsAttached() const;
    bool Reattach();
    const std::string& GetLastErrorMessage() const;

private:
    bool RefreshDesktopWindows();
    void SpawnWorkerW();
    void WriteDebugLog(const std::string& line) const;

    HWND m_progman = nullptr;
    HWND m_defView = nullptr;
    HWND m_workerW = nullptr;
    HWND m_host = nullptr;
    bool m_raisedDesktop = false;
    bool m_attached = false;
    std::string m_lastError;
};

bool IsFullscreenAppActive(HWND uiHwnd, HWND wallpaperHwnd);
bool IsDisplayOff();
bool SetStartWithWindows(bool enabled);
bool IsStartWithWindowsEnabled();

}

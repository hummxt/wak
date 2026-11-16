#include "desktop/DesktopHost.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>

namespace ge {
namespace {

constexpr LONG_PTR kNoRedirectionBitmap = 0x00200000;

bool HasExStyle(HWND hwnd, LONG_PTR style) {
    return (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & style) != 0;
}

struct EnumFindDefView {
    HWND defViewParent = nullptr;
    HWND defView = nullptr;
};

BOOL CALLBACK EnumFindDefViewProc(HWND hwnd, LPARAM lParam) {
    auto* data = reinterpret_cast<EnumFindDefView*>(lParam);
    HWND defView = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (defView) {
        data->defViewParent = hwnd;
        data->defView = defView;
        return FALSE;
    }
    return TRUE;
}

void ForceDefViewRefresh(HWND defView) {
    if (!defView) {
        return;
    }
    ShowWindow(defView, SW_HIDE);
    Sleep(1);
    ShowWindow(defView, SW_SHOW);
}

}

void DesktopHost::WriteDebugLog(const std::string& line) const {
    const std::wstring path = JoinPath(GetExecutableDirectory(), L"desktop_debug.log");
    std::ofstream file(path, std::ios::app);
    if (file) {
        file << line << '\n';
    }
    LogInfo(line);
}

void DesktopHost::SpawnWorkerW() {
    if (!m_progman) {
        return;
    }

    DWORD_PTR result = 0;
    SendMessageTimeoutW(m_progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, &result);
    SendMessageTimeoutW(m_progman, 0x052C, 0xD, 0, SMTO_NORMAL, 1000, &result);
    SendMessageTimeoutW(m_progman, 0x052C, 0xD, 1, SMTO_NORMAL, 1000, &result);
}

bool DesktopHost::RefreshDesktopWindows() {
    m_progman = FindWindowW(L"Progman", nullptr);
    m_defView = nullptr;
    m_workerW = nullptr;
    m_host = nullptr;
    m_raisedDesktop = false;
    m_lastError.clear();

    if (!m_progman) {
        m_lastError = "Progman window not found";
        WriteDebugLog(m_lastError);
        return false;
    }

    m_raisedDesktop = HasExStyle(m_progman, kNoRedirectionBitmap);
    SpawnWorkerW();

    EnumFindDefView found{};
    EnumWindows(EnumFindDefViewProc, reinterpret_cast<LPARAM>(&found));
    if (!found.defView) {
        found.defView = FindWindowExW(m_progman, nullptr, L"SHELLDLL_DefView", nullptr);
        if (found.defView) {
            found.defViewParent = m_progman;
        }
    }
    m_defView = found.defView;

    m_workerW = FindWindowExW(m_progman, nullptr, L"WorkerW", nullptr);
    if (!m_workerW && found.defViewParent) {
        m_workerW = FindWindowExW(nullptr, found.defViewParent, L"WorkerW", nullptr);
    }
    if (!m_workerW) {
        HWND next = nullptr;
        while ((next = FindWindowExW(nullptr, next, L"WorkerW", nullptr)) != nullptr) {
            if (!FindWindowExW(next, nullptr, L"SHELLDLL_DefView", nullptr)) {
                m_workerW = next;
            }
        }
    }

    if (m_raisedDesktop) {
        m_host = m_progman;
    } else if (m_workerW) {
        m_host = m_workerW;
    } else {
        m_host = m_progman;
    }

    char buffer[256]{};
    std::snprintf(
        buffer,
        sizeof(buffer),
        "Desktop refresh raised=%d progman=%p defView=%p workerW=%p host=%p",
        m_raisedDesktop ? 1 : 0,
        m_progman,
        m_defView,
        m_workerW,
        m_host
    );
    WriteDebugLog(buffer);

    if (!m_host) {
        m_lastError = "No desktop host window";
        WriteDebugLog(m_lastError);
        return false;
    }

    return true;
}

bool DesktopHost::Attach() {
    m_attached = RefreshDesktopWindows();
    return m_attached;
}

void DesktopHost::Detach() {
    m_progman = nullptr;
    m_defView = nullptr;
    m_workerW = nullptr;
    m_host = nullptr;
    m_raisedDesktop = false;
    m_attached = false;
}

bool DesktopHost::EmbedWindow(HWND hwnd) {
    if (!hwnd) {
        m_lastError = "Wallpaper hwnd is null";
        return false;
    }

    if (!Attach()) {
        return false;
    }

    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU | WS_OVERLAPPEDWINDOW | WS_POPUP);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);

    LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    ex |= WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW;
    ex &= ~(WS_EX_APPWINDOW | WS_EX_TRANSPARENT);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, ex);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

    HWND parent = m_host;
    if (!parent) {
        m_lastError = "Desktop parent missing";
        return false;
    }

    SetParent(hwnd, parent);

    RECT parentRect{};
    GetClientRect(parent, &parentRect);
    int width = parentRect.right - parentRect.left;
    int height = parentRect.bottom - parentRect.top;
    const int virtualW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int virtualH = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (width < virtualW) {
        width = virtualW;
    }
    if (height < virtualH) {
        height = virtualH;
    }
    if (width <= 1 || height <= 1) {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }

    UINT flags = SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOACTIVATE;
    if (parent == m_workerW) {
        SetWindowPos(hwnd, nullptr, 0, 0, width, height, flags | SWP_NOZORDER);
    } else if (parent == m_progman && m_defView) {
        SetWindowPos(hwnd, m_defView, 0, 0, width, height, flags);
        if (m_workerW) {
            SetWindowPos(m_workerW, hwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        SetWindowPos(m_defView, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
        SetWindowPos(hwnd, nullptr, 0, 0, width, height, flags | SWP_NOZORDER);
    }

    ForceDefViewRefresh(m_defView);

    char buffer[256]{};
    std::snprintf(
        buffer,
        sizeof(buffer),
        "Embedded wallpaper hwnd=%p parent=%p size=%dx%d raised=%d",
        hwnd,
        parent,
        width,
        height,
        m_raisedDesktop ? 1 : 0
    );
    WriteDebugLog(buffer);

    m_lastError.clear();
    return true;
}

void DesktopHost::ReleaseWindow(HWND hwnd) {
    if (!hwnd) {
        return;
    }

    SetParent(hwnd, nullptr);

    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style &= ~(WS_CHILD | WS_VISIBLE);
    style |= WS_POPUP;
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    ShowWindow(hwnd, SW_HIDE);
}

HWND DesktopHost::GetHostWindow() const {
    return m_host;
}

HWND DesktopHost::GetDefView() const {
    return m_defView;
}

HWND DesktopHost::GetProgman() const {
    return m_progman;
}

HWND DesktopHost::GetWorkerW() const {
    return m_workerW;
}

bool DesktopHost::IsRaisedDesktop() const {
    return m_raisedDesktop;
}

bool DesktopHost::IsAttached() const {
    return m_attached && m_host != nullptr;
}

bool DesktopHost::Reattach() {
    Detach();
    return Attach();
}

const std::string& DesktopHost::GetLastErrorMessage() const {
    return m_lastError;
}

bool IsFullscreenAppActive(HWND uiHwnd, HWND wallpaperHwnd) {
    HWND foreground = GetForegroundWindow();
    if (!foreground) {
        return false;
    }
    if (foreground == uiHwnd || foreground == wallpaperHwnd) {
        return false;
    }

    HWND desktop = GetDesktopWindow();
    HWND shell = GetShellWindow();
    if (foreground == desktop || foreground == shell) {
        return false;
    }

    wchar_t className[256]{};
    GetClassNameW(foreground, className, 256);
    if (wcsstr(className, L"Wakbg") != nullptr) {
        return false;
    }
    if (wcscmp(className, L"WorkerW") == 0 || wcscmp(className, L"Progman") == 0 || wcscmp(className, L"Shell_TrayWnd") == 0) {
        return false;
    }

    RECT windowRect{};
    if (!GetWindowRect(foreground, &windowRect)) {
        return false;
    }

    HMONITOR monitor = MonitorFromWindow(foreground, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) {
        return false;
    }

    return windowRect.left <= info.rcMonitor.left
        && windowRect.top <= info.rcMonitor.top
        && windowRect.right >= info.rcMonitor.right
        && windowRect.bottom >= info.rcMonitor.bottom;
}

bool IsDisplayOff() {
    return false;
}

bool SetStartWithWindows(bool enabled) {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        return false;
    }

    const wchar_t* valueName = L"Wakbg";
    bool ok = true;
    if (enabled) {
        wchar_t path[MAX_PATH]{};
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring command = L"\"";
        command += path;
        command += L"\" --background";
        ok = RegSetValueExW(
            key,
            valueName,
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(command.c_str()),
            static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t))
        ) == ERROR_SUCCESS;
    } else {
        RegDeleteValueW(key, valueName);
    }

    RegCloseKey(key);
    return ok;
}

bool IsStartWithWindowsEnabled() {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        return false;
    }

    DWORD type = 0;
    DWORD size = 0;
    const LONG result = RegQueryValueExW(key, L"Wakbg", nullptr, &type, nullptr, &size);
    RegCloseKey(key);
    return result == ERROR_SUCCESS;
}

}

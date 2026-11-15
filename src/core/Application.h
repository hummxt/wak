#pragma once

#include "core/Config.h"
#include "core/AnimationManager.h"
#include "renderer/Renderer.h"
#include "desktop/DesktopHost.h"
#include "desktop/MonitorManager.h"
#include "ui/MainWindow.h"

#include <Windows.h>
#include <shellapi.h>
#include <chrono>
#include <string>

namespace ge {

class Application {
public:
    bool Initialize(HINSTANCE instance, int nCmdShow);
    int Run();
    void Shutdown();
    void ApplyDesktopForTesting();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateAppWindow(HINSTANCE instance, int nCmdShow);
    bool CreateWallpaperWindow(HINSTANCE instance);
    bool InitializeSystems();
    void Update();
    void RenderFrame();
    void ApplyPreset(const AnimationPreset& preset);
    bool ApplyShaderToRenderer(Renderer& renderer, const AnimationPreset& preset);
    void SyncPaletteToRenderers();
    void ApplyWallpaperMode(bool enabled);
    void PositionWallpaperWindow();
    void SyncStartupSetting();
    bool ShouldPauseWallpaper() const;
    int EffectiveFpsLimit() const;
    std::wstring ResolvePath(const std::wstring& relative) const;
    void CaptureWallpaperDebugFrame();

    bool CreateTrayIcon();
    void DestroyTrayIcon();
    void ShowUiWindow();
    void HideUiWindow();
    void RequestExit();

    static constexpr UINT WM_TRAYICON = WM_APP + 1;
    static constexpr UINT ID_TRAY_SHOW = 1001;
    static constexpr UINT ID_TRAY_EXIT = 1002;

    HINSTANCE m_instance = nullptr;
    HWND m_hwnd = nullptr;
    HWND m_wallpaperHwnd = nullptr;
    NOTIFYICONDATAW m_tray{};
    bool m_trayAdded = false;
    bool m_running = false;
    bool m_exitRequested = false;
    bool m_uiVisible = true;
    bool m_uiResized = false;
    bool m_wallpaperModeApplied = false;
    bool m_startWithWindowsApplied = false;
    uint32_t m_width = 1280;
    uint32_t m_height = 720;
    uint32_t m_wallpaperWidth = 0;
    uint32_t m_wallpaperHeight = 0;

    Config m_config;
    AnimationManager m_animations;
    Renderer m_renderer;
    Renderer m_wallpaperRenderer;
    MainWindow m_ui;
    DesktopHost m_desktop;
    MonitorManager m_monitors;

    std::wstring m_baseDir;
    std::wstring m_configPath;
    std::string m_activePresetId;
    std::string m_desktopStatus;
    bool m_capturePending = false;
    int m_wallpaperFrames = 0;
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_lastFrame;
};

}

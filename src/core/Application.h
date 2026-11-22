#pragma once

#include "core/Config.h"
#include "core/AnimationManager.h"
#include "renderer/Renderer.h"
#include "renderer/ThumbnailCache.h"
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
    bool Initialize(HINSTANCE instance, int nCmdShow, bool backgroundMode = false);
    int Run();
    void Shutdown();
    void ApplyDesktopForTesting();

    // If another Wakbg is already running, ask it to show UI and return true.
    static bool ActivateExistingInstance();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateAppWindow(HINSTANCE instance, int nCmdShow);
    bool CreateWallpaperWindow(HINSTANCE instance);
    void ApplyWindowChrome();
    bool InitializeSystems();
    void ResolveConfigPath();
    void Update();
    void RenderFrame();
    void RenderWallpaperOnly();
    void ApplyPreset(const AnimationPreset& preset);
    void LoadPresetIntoAnimation(const AnimationPreset& preset);
    bool ApplyShaderToRenderer(Renderer& renderer, const AnimationPreset& preset);
    void SyncPaletteToRenderers();
    void ApplyWallpaperMode(bool enabled);
    void PositionWallpaperWindow();
    void QueryDesktopPixelSize(uint32_t& width, uint32_t& height, int& x, int& y);
    void SyncStartupSetting();
    void EnablePersistentWallpaper();
    bool TryAttachWallpaper();
    // Saves a BMP fallback. setSystemWallpaper=true also calls SPI_SETDESKWALLPAPER
    // (only safe when live embed is not active — e.g. session end / shutdown).
    bool PersistStaticWallpaper(bool setSystemWallpaper, float timeSeconds);
    void PersistForSessionEnd();
    bool ApplyStaticWallpaperFile(const std::wstring& path);
    bool ShouldPauseWallpaper() const;
    bool IsWallpaperEmbedHealthy() const;
    int EffectiveFpsLimit() const;
    std::wstring ResolvePath(const std::wstring& relative) const;
    std::wstring GetPersistedWallpaperPath() const;
    bool CaptureWallpaperToFile(const std::wstring& path);
    void CaptureWallpaperDebugFrame();

    bool CreateTrayIcon();
    void DestroyTrayIcon();
    void ShowUiWindow();
    void HideUiWindow();
    void EnterSilentBackground();
    void RequestExit(bool stopWallpaper);

    static constexpr UINT WM_TRAYICON = WM_APP + 1;
    static constexpr UINT WM_SHOW_FROM_SECOND_INSTANCE = WM_APP + 2;
    static constexpr UINT ID_TRAY_SHOW = 1001;
    static constexpr UINT ID_TRAY_EXIT = 1002;
    static constexpr UINT_PTR kSizeMoveTimerId = 42;

    HINSTANCE m_instance = nullptr;
    HWND m_hwnd = nullptr;
    HWND m_wallpaperHwnd = nullptr;
    HICON m_appIconBig = nullptr;
    HICON m_appIconSmall = nullptr;
    NOTIFYICONDATAW m_tray{};
    bool m_trayAdded = false;
    bool m_running = false;
    bool m_exitRequested = false;
    bool m_stopWallpaperOnExit = false;
    bool m_backgroundMode = false;
    bool m_uiVisible = true;
    bool m_uiResized = false;
    bool m_inSizeMove = false;
    bool m_wallpaperModeApplied = false;
    bool m_startWithWindowsApplied = false;
    bool m_persistWallpaperPending = false;
    bool m_wasFullscreenPaused = false;
    int m_wallpaperAttachAttempts = 0;
    uint32_t m_width = 1280;
    uint32_t m_height = 720;
    uint32_t m_wallpaperWidth = 0;
    uint32_t m_wallpaperHeight = 0;

    Config m_config;
    AnimationManager m_animations;
    Renderer m_renderer;
    Renderer m_wallpaperRenderer;
    ThumbnailCache m_thumbnails;
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
    std::chrono::steady_clock::time_point m_nextWallpaperAttachTry{};
};

}

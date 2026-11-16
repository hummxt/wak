#include "core/Application.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"
#include "utils/TextureLoader.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

#include <dwmapi.h>
#include <shellapi.h>

#include <algorithm>
#include <fstream>
#include <thread>
#include <vector>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ge {

bool Application::Initialize(HINSTANCE instance, int nCmdShow, bool backgroundMode) {
    m_instance = instance;
    m_backgroundMode = backgroundMode;
    m_baseDir = GetExecutableDirectory();
    ResolveConfigPath();
    m_config.ApplyDefaults();
    m_config.Load(m_configPath);
    if (m_config.Get().application.wallpaperMode) {
        m_config.Get().application.startWithWindows = true;
    }

    if (!CreateAppWindow(instance, backgroundMode ? SW_HIDE : nCmdShow)) {
        return false;
    }

    if (!CreateWallpaperWindow(instance)) {
        return false;
    }

    if (!InitializeSystems()) {
        return false;
    }

    CreateTrayIcon();

    if (backgroundMode) {
        HideUiWindow();
    } else {
        m_uiVisible = true;
        ShowWindow(m_hwnd, SW_SHOW);
        SetForegroundWindow(m_hwnd);
    }

    m_startTime = std::chrono::steady_clock::now();
    m_lastFrame = m_startTime;
    m_running = true;
    return true;
}

void Application::ResolveConfigPath() {
    const std::wstring appData = GetAppDataDirectory();
    EnsureDirectoryExists(appData);
    m_configPath = JoinPath(appData, L"config.json");
    if (!FileExists(m_configPath)) {
        const std::wstring bundled = ResolvePath(L"config\\default.json");
        if (FileExists(bundled)) {
            WriteTextFile(m_configPath, ReadTextFile(bundled));
        }
    }
}

std::wstring Application::ResolvePath(const std::wstring& relative) const {
    return JoinPath(m_baseDir, relative);
}

bool Application::CreateAppWindow(HINSTANCE instance, int nCmdShow) {
    const wchar_t* className = L"WakbgWindow";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0x0A, 0x0F, 0x1D));
    wc.lpszClassName = className;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        0,
        className,
        L"Wakbg",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<int>(m_width),
        static_cast<int>(m_height),
        nullptr,
        nullptr,
        instance,
        this
    );

    if (!m_hwnd) {
        LogError("Failed to create window");
        return false;
    }

    ApplyWindowChrome();
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    return true;
}

void Application::ApplyWindowChrome() {
    if (!m_hwnd) {
        return;
    }

    const std::wstring logoPath = ResolvePath(L"assets\\logo\\logo.png");
    if (m_appIconBig) {
        DestroyIcon(m_appIconBig);
        m_appIconBig = nullptr;
    }
    if (m_appIconSmall) {
        DestroyIcon(m_appIconSmall);
        m_appIconSmall = nullptr;
    }

    // Full-size logo with rounded corners only (no shrink/padding).
    m_appIconBig = LoadIconFromFile(logoPath, 256, 256, 0.0f, 0.22f);
    m_appIconSmall = LoadIconFromFile(logoPath, 32, 32, 0.0f, 0.22f);
    if (m_appIconBig) {
        SendMessageW(m_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(m_appIconBig));
    }
    if (m_appIconSmall) {
        SendMessageW(m_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_appIconSmall));
    }

    const BOOL dark = TRUE;
    DwmSetWindowAttribute(m_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

    const COLORREF caption = RGB(0x0A, 0x0F, 0x1D);
    const COLORREF border = RGB(0x04, 0x07, 0x0E);
    const COLORREF text = RGB(0xF2, 0xF4, 0xF8);
    DwmSetWindowAttribute(m_hwnd, DWMWA_CAPTION_COLOR, &caption, sizeof(caption));
    DwmSetWindowAttribute(m_hwnd, DWMWA_BORDER_COLOR, &border, sizeof(border));
    DwmSetWindowAttribute(m_hwnd, DWMWA_TEXT_COLOR, &text, sizeof(text));
}

bool Application::CreateWallpaperWindow(HINSTANCE instance) {
    const wchar_t* className = L"WakbgWallpaper";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = className;
    RegisterClassExW(&wc);

    int x = 0;
    int y = 0;
    QueryDesktopPixelSize(m_wallpaperWidth, m_wallpaperHeight, x, y);

    m_wallpaperHwnd = CreateWindowExW(
        WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
        className,
        L"Wakbg Wallpaper",
        WS_POPUP,
        x,
        y,
        static_cast<int>(m_wallpaperWidth),
        static_cast<int>(m_wallpaperHeight),
        nullptr,
        nullptr,
        instance,
        this
    );

    if (!m_wallpaperHwnd) {
        LogError("Failed to create wallpaper window");
        return false;
    }

    ShowWindow(m_wallpaperHwnd, SW_HIDE);
    return true;
}

bool Application::CreateTrayIcon() {
    m_tray = {};
    m_tray.cbSize = sizeof(m_tray);
    m_tray.hWnd = m_hwnd;
    m_tray.uID = 1;
    m_tray.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_tray.uCallbackMessage = WM_TRAYICON;
    m_tray.hIcon = m_appIconSmall ? m_appIconSmall : LoadIcon(nullptr, IDI_APPLICATION);
    wcsncpy_s(m_tray.szTip, L"Wakbg", _TRUNCATE);
    m_trayAdded = Shell_NotifyIconW(NIM_ADD, &m_tray) == TRUE;
    return m_trayAdded;
}

void Application::DestroyTrayIcon() {
    if (m_trayAdded) {
        Shell_NotifyIconW(NIM_DELETE, &m_tray);
        m_trayAdded = false;
    }
}

void Application::ShowUiWindow() {
    m_uiVisible = true;
    ShowWindow(m_hwnd, SW_RESTORE);
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}

void Application::HideUiWindow() {
    m_uiVisible = false;
    ShowWindow(m_hwnd, SW_HIDE);
    m_desktopStatus = "UI hidden. Wallpaper keeps running. Tray icon: Open / Exit.";
}

void Application::RequestExit(bool stopWallpaper) {
    m_stopWallpaperOnExit = stopWallpaper;
    m_exitRequested = true;
    m_running = false;
    PostQuitMessage(0);
}

void Application::EnablePersistentWallpaper() {
    m_config.Get().application.wallpaperMode = true;
    m_config.Get().application.startWithWindows = true;
    m_config.Get().performance.renderScale = 1.0f;
    m_startWithWindowsApplied = true;
    SetStartWithWindows(true);
    m_config.Save(m_configPath);
}

std::wstring Application::GetPersistedWallpaperPath() const {
    return JoinPath(GetAppDataDirectory(), L"current_wallpaper.bmp");
}

bool Application::CaptureWallpaperToFile(const std::wstring& path) {
    if (!m_wallpaperHwnd) {
        return false;
    }

    RECT rect{};
    GetClientRect(m_wallpaperHwnd, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0) {
        return false;
    }

    HDC windowDc = GetDC(m_wallpaperHwnd);
    if (!windowDc) {
        return false;
    }
    HDC memDc = CreateCompatibleDC(windowDc);
    HBITMAP bitmap = CreateCompatibleBitmap(windowDc, width, height);
    HGDIOBJ old = SelectObject(memDc, bitmap);
    BitBlt(memDc, 0, 0, width, height, windowDc, 0, 0, SRCCOPY);

    BITMAPINFOHEADER info{};
    info.biSize = sizeof(info);
    info.biWidth = width;
    info.biHeight = -height;
    info.biPlanes = 1;
    info.biBitCount = 24;
    info.biCompression = BI_RGB;

    const int stride = ((width * 3 + 3) / 4) * 4;
    std::vector<unsigned char> pixels(static_cast<size_t>(stride * height));
    GetDIBits(memDc, bitmap, 0, height, pixels.data(), reinterpret_cast<BITMAPINFO*>(&info), DIB_RGB_COLORS);

    EnsureDirectoryExists(GetAppDataDirectory());
    std::ofstream file(path, std::ios::binary);
    bool ok = false;
    if (file) {
        BITMAPFILEHEADER header{};
        header.bfType = 0x4D42;
        header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        header.bfSize = header.bfOffBits + static_cast<DWORD>(pixels.size());
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(&info), sizeof(info));
        file.write(reinterpret_cast<const char*>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
        ok = static_cast<bool>(file);
    }

    SelectObject(memDc, old);
    DeleteObject(bitmap);
    DeleteDC(memDc);
    ReleaseDC(m_wallpaperHwnd, windowDc);
    return ok;
}

bool Application::PersistStaticWallpaper() {
    const std::wstring path = GetPersistedWallpaperPath();
    SyncPaletteToRenderers();
    for (int i = 0; i < 3; ++i) {
        m_wallpaperRenderer.Render(m_config.Get().animation, 1.25f, true, false);
    }

    if (!CaptureWallpaperToFile(path)) {
        LogWarning("Failed to save persistent wallpaper image");
        return false;
    }

    wchar_t absolute[MAX_PATH]{};
    if (GetFullPathNameW(path.c_str(), MAX_PATH, absolute, nullptr) == 0) {
        wcsncpy_s(absolute, path.c_str(), _TRUNCATE);
    }

    const BOOL ok = SystemParametersInfoW(
        SPI_SETDESKWALLPAPER,
        0,
        absolute,
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
    );
    if (!ok) {
        LogWarning("Failed to set Windows desktop wallpaper fallback");
        return false;
    }

    LogInfo("Saved persistent wallpaper fallback");
    return true;
}

bool Application::TryAttachWallpaper() {
    if (!m_config.Get().application.wallpaperMode || !m_wallpaperHwnd) {
        return false;
    }

    if (!m_desktop.EmbedWindow(m_wallpaperHwnd)) {
        m_wallpaperModeApplied = false;
        m_desktopStatus = "Waiting for Windows desktop to become ready...";
        return false;
    }

    PositionWallpaperWindow();
    ShowWindow(m_wallpaperHwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(m_wallpaperHwnd);
    m_wallpaperModeApplied = true;
    m_wallpaperAttachAttempts = 0;
    m_desktopStatus = "Desktop wallpaper running and will restore after reboot.";
    LogInfo(m_desktopStatus);
    m_persistWallpaperPending = true;
    m_wallpaperFrames = 0;
    return true;
}

void Application::QueryDesktopPixelSize(uint32_t& width, uint32_t& height, int& x, int& y) {
    m_monitors.Refresh();
    x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    long w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    long h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    const MonitorInfo* primary = m_monitors.GetPrimary();
    if (primary && primary->width > 0 && primary->height > 0) {
        w = (std::max)(w, static_cast<long>(primary->width));
        h = (std::max)(h, static_cast<long>(primary->height));
    }

    if (w <= 1 || h <= 1) {
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);
        x = 0;
        y = 0;
    }

    width = static_cast<uint32_t>((std::max)(1L, w));
    height = static_cast<uint32_t>((std::max)(1L, h));
}

bool Application::InitializeSystems() {
    m_monitors.Refresh();

    if (!m_renderer.Initialize(m_hwnd, m_width, m_height, ResolvePath(L"shaders"), false)) {
        return false;
    }

    m_desktop.Attach();
    const bool useComposition = m_desktop.IsRaisedDesktop();
    if (!m_wallpaperRenderer.Initialize(m_wallpaperHwnd, m_wallpaperWidth, m_wallpaperHeight, ResolvePath(L"shaders"), true, useComposition)) {
        return false;
    }

    if (!m_ui.Initialize(m_hwnd, m_renderer.GetDevice())) {
        return false;
    }

    const std::wstring presetsDir = ResolvePath(L"library\\presets");
    if (!m_animations.LoadPresets(presetsDir)) {
        LogWarning("No animation presets loaded");
    }

    const AnimationPreset* preset = m_animations.FindById(m_config.Get().wallpaper.selected);
    if (!preset && !m_animations.GetPresets().empty()) {
        preset = &m_animations.GetPresets().front();
    }
    if (preset) {
        ApplyShaderToRenderer(m_renderer, *preset);
        ApplyShaderToRenderer(m_wallpaperRenderer, *preset);
        m_activePresetId = preset->id;
        m_ui.GetLibraryState().selectedPresetId = preset->id;
        m_config.Get().wallpaper.selected = preset->id;
        SyncPaletteToRenderers();
    }

    m_wallpaperModeApplied = false;
    if (m_config.Get().application.wallpaperMode || m_backgroundMode) {
        if (m_backgroundMode) {
            m_config.Get().application.wallpaperMode = true;
        }
        EnablePersistentWallpaper();
        if (!TryAttachWallpaper()) {
            m_nextWallpaperAttachTry = std::chrono::steady_clock::now() + std::chrono::seconds(1);
            LogWarning("Desktop not ready yet; will retry wallpaper attach");
        }
    }

    m_startWithWindowsApplied = m_config.Get().application.startWithWindows;
    SyncStartupSetting();
    return true;
}

void Application::SyncPaletteToRenderers() {
    const auto& anim = m_config.Get().animation;
    m_renderer.SetPalette(anim.colorA, anim.colorB, anim.colorC, anim.colorD);
    m_wallpaperRenderer.SetPalette(anim.colorA, anim.colorB, anim.colorC, anim.colorD);
}

bool Application::ApplyShaderToRenderer(Renderer& renderer, const AnimationPreset& preset) {
    std::wstring shaderPath = Widen(preset.shader);
    for (auto& ch : shaderPath) {
        if (ch == L'/') {
            ch = L'\\';
        }
    }

    if (!renderer.SetShader(preset.id, shaderPath)) {
        LogError("Failed to apply shader for preset: " + preset.id);
        return false;
    }

    return true;
}

void Application::ApplyDesktopForTesting() {
    const AnimationPreset* preset = m_animations.FindById(m_config.Get().wallpaper.selected);
    if (!preset && !m_animations.GetPresets().empty()) {
        preset = &m_animations.GetPresets().front();
    }
    if (preset) {
        ApplyPreset(*preset);
        m_capturePending = true;
        m_wallpaperFrames = 0;
        HideUiWindow();
    } else {
        LogError("No preset available for desktop test apply");
    }
}

void Application::CaptureWallpaperDebugFrame() {
    CaptureWallpaperToFile(ResolvePath(L"wallpaper_capture.bmp"));
}

void Application::ApplyPreset(const AnimationPreset& preset) {
    if (!ApplyShaderToRenderer(m_renderer, preset)) {
        return;
    }
    if (!ApplyShaderToRenderer(m_wallpaperRenderer, preset)) {
        return;
    }

    m_activePresetId = preset.id;
    m_config.Get().wallpaper.selected = preset.id;
    m_config.Get().animation.speed = preset.defaultSpeed;
    m_config.Get().animation.intensity = preset.defaultIntensity;
    m_config.Get().animation.angle = preset.defaultAngle;
    m_config.Get().animation.colorA = preset.colorA;
    m_config.Get().animation.colorB = preset.colorB;
    m_config.Get().animation.colorC = preset.colorC;
    m_config.Get().animation.colorD = preset.colorD;
    m_animations.PushRecent(preset.id);
    SyncPaletteToRenderers();
    m_config.Save(m_configPath);
    ApplyWallpaperMode(true);
}

void Application::PositionWallpaperWindow() {
    if (!m_wallpaperHwnd || !m_desktop.GetHostWindow()) {
        return;
    }

    int originX = 0;
    int originY = 0;
    QueryDesktopPixelSize(m_wallpaperWidth, m_wallpaperHeight, originX, originY);

    HWND host = m_desktop.GetHostWindow();
    RECT hostRect{};
    GetClientRect(host, &hostRect);
    long hostW = hostRect.right - hostRect.left;
    long hostH = hostRect.bottom - hostRect.top;

    HMONITOR monitor = MonitorFromWindow(host, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfoW(monitor, &monitorInfo)) {
        hostW = (std::max)(hostW, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
        hostH = (std::max)(hostH, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);
    }

    m_wallpaperWidth = static_cast<uint32_t>((std::max)(1L, (std::max)(hostW, static_cast<long>(m_wallpaperWidth))));
    m_wallpaperHeight = static_cast<uint32_t>((std::max)(1L, (std::max)(hostH, static_cast<long>(m_wallpaperHeight))));

    UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED;
    if (host == m_desktop.GetProgman() && m_desktop.GetDefView()) {
        SetWindowPos(
            m_wallpaperHwnd,
            m_desktop.GetDefView(),
            0,
            0,
            static_cast<int>(m_wallpaperWidth),
            static_cast<int>(m_wallpaperHeight),
            flags
        );
        if (m_desktop.GetWorkerW()) {
            SetWindowPos(m_desktop.GetWorkerW(), m_wallpaperHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        SetWindowPos(m_desktop.GetDefView(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
        SetWindowPos(
            m_wallpaperHwnd,
            nullptr,
            0,
            0,
            static_cast<int>(m_wallpaperWidth),
            static_cast<int>(m_wallpaperHeight),
            flags | SWP_NOZORDER
        );
    }

    RECT clientRect{};
    GetClientRect(m_wallpaperHwnd, &clientRect);
    const long clientW = clientRect.right - clientRect.left;
    const long clientH = clientRect.bottom - clientRect.top;
    if (clientW > 1 && clientH > 1) {
        m_wallpaperWidth = static_cast<uint32_t>(clientW);
        m_wallpaperHeight = static_cast<uint32_t>(clientH);
    }

    m_wallpaperRenderer.Resize(m_wallpaperWidth, m_wallpaperHeight);
    m_wallpaperRenderer.RecreateDeviceSurface();
    SyncPaletteToRenderers();
    LogInfo("Wallpaper surface " + std::to_string(m_wallpaperWidth) + "x" + std::to_string(m_wallpaperHeight));
}

void Application::ApplyWallpaperMode(bool enabled) {
    m_config.Get().application.wallpaperMode = enabled;

    if (enabled) {
        EnablePersistentWallpaper();
        if (!TryAttachWallpaper()) {
            m_wallpaperModeApplied = false;
            m_nextWallpaperAttachTry = std::chrono::steady_clock::now() + std::chrono::seconds(2);
            m_desktopStatus = "Wallpaper saved. Waiting for desktop attach...";
            LogWarning(m_desktopStatus);
        }
    } else {
        m_desktop.ReleaseWindow(m_wallpaperHwnd);
        m_desktop.Detach();
        m_wallpaperModeApplied = false;
        m_persistWallpaperPending = false;
        m_desktopStatus = "Desktop wallpaper disabled";
        SetStartWithWindows(false);
        m_config.Get().application.startWithWindows = false;
        m_startWithWindowsApplied = false;
    }

    m_config.Save(m_configPath);
}

void Application::SyncStartupSetting() {
    SetStartWithWindows(m_config.Get().application.startWithWindows);
}

bool Application::ShouldPauseWallpaper() const {
    if (!m_config.Get().performance.pauseWhenFullscreen) {
        return false;
    }
    return IsFullscreenAppActive(m_hwnd, m_wallpaperHwnd);
}

int Application::EffectiveFpsLimit() const {
    int fps = m_config.Get().performance.fpsLimit;
    if (fps < 15) {
        fps = 30;
    }
    if (m_uiVisible) {
        return (std::max)(fps, 60);
    }
    if (m_config.Get().performance.powerSaving) {
        fps = (std::min)(fps, 20);
    }
    if (m_config.Get().animation.speed <= 0.0001f) {
        fps = (std::min)(fps, 8);
    }
    return fps;
}

int Application::Run() {
    MSG msg{};
    while (m_running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (!m_running) {
            break;
        }

        if (m_monitors.HasDisplayChanged()) {
            m_monitors.ClearDisplayChanged();
            if (m_config.Get().application.wallpaperMode) {
                m_desktop.Reattach();
                m_desktop.EmbedWindow(m_wallpaperHwnd);
                PositionWallpaperWindow();
            }
        }

        Update();
        RenderFrame();
    }
    return 0;
}

void Application::Update() {
    if (m_uiResized && IsWindowVisible(m_hwnd)) {
        RECT rect{};
        GetClientRect(m_hwnd, &rect);
        m_width = static_cast<uint32_t>((std::max)(1L, rect.right - rect.left));
        m_height = static_cast<uint32_t>((std::max)(1L, rect.bottom - rect.top));
        m_renderer.Resize(m_width, m_height);
        m_uiResized = false;
    }

    const bool wantWallpaper = m_config.Get().application.wallpaperMode;
    if (wantWallpaper && !m_wallpaperModeApplied) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= m_nextWallpaperAttachTry) {
            ++m_wallpaperAttachAttempts;
            if (!TryAttachWallpaper()) {
                const int delaySec = (std::min)(10, 1 + m_wallpaperAttachAttempts / 2);
                m_nextWallpaperAttachTry = now + std::chrono::seconds(delaySec);
            }
        }
    } else if (!wantWallpaper && m_wallpaperModeApplied) {
        ApplyWallpaperMode(false);
    }

    const bool wantStartup = m_config.Get().application.startWithWindows;
    if (wantStartup != m_startWithWindowsApplied) {
        SyncStartupSetting();
        m_startWithWindowsApplied = wantStartup;
    }
}

void Application::RenderFrame() {
    const auto now = std::chrono::steady_clock::now();
    const float elapsed = std::chrono::duration<float>(now - m_startTime).count();

    const bool pauseWallpaper = ShouldPauseWallpaper();
    const bool drawUi = m_uiVisible && m_hwnd && IsWindowVisible(m_hwnd);

    if (m_config.Get().application.wallpaperMode && m_wallpaperHwnd && m_wallpaperModeApplied && !pauseWallpaper) {
        m_wallpaperRenderer.SetHighQuality(!drawUi);
        const bool renderWallpaperThisFrame = !drawUi || (m_wallpaperFrames % 3 == 0) || m_persistWallpaperPending || m_capturePending;
        if (renderWallpaperThisFrame) {
            SyncPaletteToRenderers();
            m_wallpaperRenderer.Render(m_config.Get().animation, elapsed, true, false);
        }
        ++m_wallpaperFrames;
        if (m_persistWallpaperPending && m_wallpaperFrames >= 12) {
            PersistStaticWallpaper();
            m_persistWallpaperPending = false;
        }
        if (m_capturePending && m_wallpaperFrames >= 15) {
            CaptureWallpaperDebugFrame();
            m_capturePending = false;
        }
    }

    if (drawUi) {
        const bool scrolling = ImGui::GetIO().MouseWheel != 0.0f || ImGui::GetIO().MouseWheelH != 0.0f;
        if (!scrolling && m_thumbnails.HasMissing(m_animations.GetPresets())) {
            m_thumbnails.BuildMissing(m_renderer, m_animations.GetPresets(), 1);
        }

        if (!m_activePresetId.empty()) {
            m_renderer.ActivateShader(m_activePresetId);
        }
        SyncPaletteToRenderers();
        uint32_t previewW = 640;
        uint32_t previewH = 360;
        if (m_wallpaperWidth > 0 && m_wallpaperHeight > 0) {
            previewH = 360;
            previewW = static_cast<uint32_t>((360.0f * static_cast<float>(m_wallpaperWidth)) / static_cast<float>(m_wallpaperHeight));
            if (previewW < 320) {
                previewW = 320;
            }
            if (previewW > 960) {
                previewW = 960;
                previewH = static_cast<uint32_t>((960.0f * static_cast<float>(m_wallpaperHeight)) / static_cast<float>(m_wallpaperWidth));
            }
        }
        m_renderer.EnsurePreviewTarget(previewW, previewH);
        m_renderer.RenderPreview(m_config.Get().animation, elapsed);

        m_renderer.GetDevice().BeginFrame(0.039f, 0.059f, 0.114f, 1.0f);
        m_ui.BeginFrame();
        m_ui.Draw(
            m_config,
            m_animations,
            m_renderer.GetPreviewSRV(),
            [this](const std::string& id) -> ID3D11ShaderResourceView* {
                return m_thumbnails.Get(id);
            },
            m_desktopStatus,
            [this](const AnimationPreset& preset) {
                ApplyShaderToRenderer(m_renderer, preset);
                ApplyShaderToRenderer(m_wallpaperRenderer, preset);
                m_activePresetId = preset.id;
                m_config.Get().wallpaper.selected = preset.id;
                m_config.Get().animation.speed = preset.defaultSpeed;
                m_config.Get().animation.intensity = preset.defaultIntensity;
                m_config.Get().animation.angle = preset.defaultAngle;
                m_config.Get().animation.colorA = preset.colorA;
                m_config.Get().animation.colorB = preset.colorB;
                m_config.Get().animation.colorC = preset.colorC;
                m_config.Get().animation.colorD = preset.colorD;
                SyncPaletteToRenderers();
                m_config.Save(m_configPath);
            },
            [this](const AnimationPreset& preset) {
                ApplyPreset(preset);
            },
            [this]() {
                SyncPaletteToRenderers();
                m_config.Save(m_configPath);
            },
            true
        );
        m_ui.EndFrame();
        m_renderer.GetDevice().Present(false);
    }

    const int fps = EffectiveFpsLimit();
    const auto frameBudget = std::chrono::milliseconds(1000 / fps);
    const auto frameEnd = std::chrono::steady_clock::now();
    const auto spent = frameEnd - m_lastFrame;
    if (spent < frameBudget) {
        std::this_thread::sleep_for(frameBudget - spent);
    }
    m_lastFrame = std::chrono::steady_clock::now();
}

void Application::Shutdown() {
    DestroyTrayIcon();
    if (m_stopWallpaperOnExit) {
        ApplyWallpaperMode(false);
    } else {
        if (m_config.Get().application.wallpaperMode) {
            EnablePersistentWallpaper();
        }
        m_config.Save(m_configPath);
    }
    m_ui.Shutdown();
    m_wallpaperRenderer.Shutdown();
    m_renderer.Shutdown();
    if (m_wallpaperHwnd) {
        DestroyWindow(m_wallpaperHwnd);
        m_wallpaperHwnd = nullptr;
    }
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    if (m_appIconBig) {
        DestroyIcon(m_appIconBig);
        m_appIconBig = nullptr;
    }
    if (m_appIconSmall) {
        DestroyIcon(m_appIconSmall);
        m_appIconSmall = nullptr;
    }
}

LRESULT CALLBACK Application::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* app = static_cast<Application*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    auto* app = reinterpret_cast<Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (app) {
        return app->HandleMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Application::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (hwnd == m_hwnd && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
        return true;
    }

    switch (msg) {
    case WM_SIZE:
        if (hwnd == m_hwnd && wParam != SIZE_MINIMIZED) {
            m_uiResized = true;
        }
        return 0;
    case WM_DISPLAYCHANGE:
        m_monitors.NotifyDisplayChanged();
        return 0;
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP) {
            ShowUiWindow();
            return 0;
        }
        if (lParam == WM_RBUTTONUP) {
            POINT pt{};
            GetCursorPos(&pt);
            HMENU menu = CreatePopupMenu();
            AppendMenuW(menu, MF_STRING, ID_TRAY_SHOW, L"Open Wakbg");
            AppendMenuW(menu, MF_STRING, ID_TRAY_EXIT, L"Stop Wallpaper and Exit");
            SetForegroundWindow(m_hwnd);
            TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwnd, nullptr);
            DestroyMenu(menu);
            return 0;
        }
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_SHOW) {
            ShowUiWindow();
            return 0;
        }
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            RequestExit(true);
            return 0;
        }
        break;
    case WM_KEYDOWN:
        if (hwnd != m_hwnd) {
            break;
        }
        if (wParam == VK_ESCAPE) {
            HideUiWindow();
            return 0;
        }
        break;
    case WM_QUERYENDSESSION:
        if (m_config.Get().application.wallpaperMode) {
            EnablePersistentWallpaper();
        }
        return TRUE;
    case WM_ENDSESSION:
        if (wParam) {
            if (m_config.Get().application.wallpaperMode) {
                EnablePersistentWallpaper();
            }
            m_stopWallpaperOnExit = false;
        }
        return 0;
    case WM_CLOSE:
        if (hwnd == m_hwnd) {
            HideUiWindow();
            if (m_config.Get().application.wallpaperMode) {
                EnablePersistentWallpaper();
            }
            return 0;
        }
        break;
    case WM_DESTROY:
        if (hwnd == m_hwnd) {
            m_hwnd = nullptr;
            if (m_exitRequested) {
                m_running = false;
                PostQuitMessage(0);
            }
        } else if (hwnd == m_wallpaperHwnd) {
            m_wallpaperHwnd = nullptr;
        }
        return 0;
    default:
        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

}

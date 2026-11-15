#include "core/Application.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

#include <shellapi.h>

#include <algorithm>
#include <fstream>
#include <thread>
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ge {

bool Application::Initialize(HINSTANCE instance, int nCmdShow) {
    m_instance = instance;
    m_baseDir = GetExecutableDirectory();
    m_configPath = ResolvePath(L"config\\default.json");
    m_config.ApplyDefaults();
    m_config.Load(m_configPath);
    m_config.Get().application.startWithWindows = IsStartWithWindowsEnabled();

    if (!CreateAppWindow(instance, nCmdShow)) {
        return false;
    }

    if (!CreateWallpaperWindow(instance)) {
        return false;
    }

    if (!InitializeSystems()) {
        return false;
    }

    CreateTrayIcon();

    m_startTime = std::chrono::steady_clock::now();
    m_lastFrame = m_startTime;
    m_running = true;
    return true;
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
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
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

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    return true;
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

    m_monitors.Refresh();
    const MonitorInfo* primary = m_monitors.GetPrimary();
    const int x = primary ? primary->rect.left : 0;
    const int y = primary ? primary->rect.top : 0;
    m_wallpaperWidth = primary ? primary->width : 1920;
    m_wallpaperHeight = primary ? primary->height : 1080;

    m_wallpaperHwnd = CreateWindowExW(
        WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
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

    SetLayeredWindowAttributes(m_wallpaperHwnd, 0, 255, LWA_ALPHA);
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
    m_tray.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
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

void Application::RequestExit() {
    m_exitRequested = true;
    m_running = false;
    PostQuitMessage(0);
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
        if (m_config.Get().animation.speed <= 0.0f) {
            m_config.Get().animation.speed = preset->defaultSpeed;
        }
        if (m_config.Get().animation.intensity <= 0.0f) {
            m_config.Get().animation.intensity = preset->defaultIntensity;
        }
        SyncPaletteToRenderers();
    }

    m_wallpaperModeApplied = false;
    if (m_config.Get().application.wallpaperMode) {
        ApplyWallpaperMode(true);
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
    if (!m_wallpaperHwnd) {
        return;
    }

    RECT rect{};
    GetClientRect(m_wallpaperHwnd, &rect);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    HDC windowDc = GetDC(m_wallpaperHwnd);
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

    const std::wstring path = ResolvePath(L"wallpaper_capture.bmp");
    std::ofstream file(path, std::ios::binary);
    if (file) {
        BITMAPFILEHEADER header{};
        header.bfType = 0x4D42;
        header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        header.bfSize = header.bfOffBits + static_cast<DWORD>(pixels.size());
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(&info), sizeof(info));
        file.write(reinterpret_cast<const char*>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
    }

    SelectObject(memDc, old);
    DeleteObject(bitmap);
    DeleteDC(memDc);
    ReleaseDC(m_wallpaperHwnd, windowDc);
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
    m_animations.PushRecent(preset.id);
    SyncPaletteToRenderers();
    m_config.Save(m_configPath);
    ApplyWallpaperMode(true);
}

void Application::PositionWallpaperWindow() {
    if (!m_wallpaperHwnd || !m_desktop.GetHostWindow()) {
        return;
    }

    RECT hostRect{};
    GetClientRect(m_desktop.GetHostWindow(), &hostRect);
    long width = hostRect.right - hostRect.left;
    long height = hostRect.bottom - hostRect.top;
    if (width <= 1 || height <= 1) {
        width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }
    m_wallpaperWidth = static_cast<uint32_t>((std::max)(1L, width));
    m_wallpaperHeight = static_cast<uint32_t>((std::max)(1L, height));

    UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED;
    if (m_desktop.GetHostWindow() == m_desktop.GetProgman() && m_desktop.GetDefView()) {
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

    m_wallpaperRenderer.Resize(m_wallpaperWidth, m_wallpaperHeight);
    m_wallpaperRenderer.RecreateDeviceSurface();
    SyncPaletteToRenderers();
}

void Application::ApplyWallpaperMode(bool enabled) {
    m_config.Get().application.wallpaperMode = enabled;
    m_wallpaperModeApplied = enabled;

    if (enabled) {
        if (!m_desktop.EmbedWindow(m_wallpaperHwnd)) {
            const std::string err = m_desktop.GetLastErrorMessage().empty()
                ? "Could not attach wallpaper to desktop"
                : m_desktop.GetLastErrorMessage();
            LogError(err);
            m_desktopStatus = "Desktop apply failed: " + err;
            m_config.Get().application.wallpaperMode = false;
            m_wallpaperModeApplied = false;
            return;
        }

        PositionWallpaperWindow();
        ShowWindow(m_wallpaperHwnd, SW_SHOWNOACTIVATE);
        UpdateWindow(m_wallpaperHwnd);
        m_desktopStatus = "Desktop wallpaper running. Close UI anytime — use tray Exit to stop.";
        LogInfo(m_desktopStatus);
    } else {
        m_desktop.ReleaseWindow(m_wallpaperHwnd);
        m_desktop.Detach();
        m_desktopStatus = "Desktop wallpaper disabled";
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
    if (m_config.Get().performance.powerSaving) {
        fps = (std::min)(fps, 20);
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
    if (wantWallpaper != m_wallpaperModeApplied) {
        ApplyWallpaperMode(wantWallpaper);
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

    if (m_config.Get().application.wallpaperMode && m_wallpaperHwnd && !pauseWallpaper) {
        SyncPaletteToRenderers();
        m_wallpaperRenderer.Render(m_config.Get().animation, elapsed, true, false);
        ++m_wallpaperFrames;
        if (m_capturePending && m_wallpaperFrames >= 15) {
            CaptureWallpaperDebugFrame();
            m_capturePending = false;
        }
    }

    const bool drawUi = m_uiVisible && m_hwnd && IsWindowVisible(m_hwnd);
    if (drawUi) {
        SyncPaletteToRenderers();
        m_renderer.EnsurePreviewTarget(960, 540);
        m_renderer.RenderPreview(m_config.Get().animation, elapsed);

        m_renderer.GetDevice().BeginFrame(0.06f, 0.07f, 0.09f, 1.0f);
        m_ui.BeginFrame();
        m_ui.Draw(
            m_config,
            m_animations,
            m_renderer.GetPreviewSRV(),
            m_desktopStatus,
            [this](const AnimationPreset& preset) {
                ApplyShaderToRenderer(m_renderer, preset);
                ApplyShaderToRenderer(m_wallpaperRenderer, preset);
                m_activePresetId = preset.id;
                m_config.Get().animation.colorA = preset.colorA;
                m_config.Get().animation.colorB = preset.colorB;
                m_config.Get().animation.colorC = preset.colorC;
                m_config.Get().animation.colorD = preset.colorD;
                SyncPaletteToRenderers();
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
    m_config.Save(m_configPath);
    ApplyWallpaperMode(false);
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
            RequestExit();
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
    case WM_CLOSE:
        if (hwnd == m_hwnd) {
            HideUiWindow();
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

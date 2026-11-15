#pragma once

#include "core/AnimationManager.h"
#include "core/Config.h"
#include "renderer/Renderer.h"
#include "ui/LibraryView.h"
#include "ui/PreviewView.h"
#include "ui/SettingsView.h"

#include <d3d11.h>
#include <functional>
#include <string>

namespace ge {

class MainWindow {
public:
    bool Initialize(HWND hwnd, GraphicsDevice& device);
    void Shutdown();
    void BeginFrame();
    void Draw(
        Config& config,
        AnimationManager& animations,
        ID3D11ShaderResourceView* previewSrv,
        const std::string& desktopStatus,
        const std::function<void(const AnimationPreset&)>& onSelectPreset,
        const std::function<void(const AnimationPreset&)>& onApplyPreset,
        const std::function<void()>& onSettingsChanged,
        bool uiVisible
    );
    void EndFrame();
    bool WantsCaptureKeyboard() const;
    bool WantsCaptureMouse() const;

    LibraryViewState& GetLibraryState();

private:
    LibraryView m_libraryView;
    PreviewView m_previewView;
    SettingsView m_settingsView;
    LibraryViewState m_libraryState;
    bool m_initialized = false;
};

}

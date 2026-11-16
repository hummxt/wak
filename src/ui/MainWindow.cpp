#include "ui/MainWindow.h"
#include "ui/Theme.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace ge {

bool MainWindow::Initialize(HWND hwnd, GraphicsDevice& device) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ApplyDarkTheme();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device.GetDevice(), device.GetContext());
    m_initialized = true;
    return true;
}

void MainWindow::Shutdown() {
    if (!m_initialized) {
        return;
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

void MainWindow::BeginFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void MainWindow::Draw(
    Config& config,
    AnimationManager& animations,
    ID3D11ShaderResourceView* previewSrv,
    const std::string& desktopStatus,
    const std::function<void(const AnimationPreset&)>& onSelectPreset,
    const std::function<void(const AnimationPreset&)>& onApplyPreset,
    const std::function<void()>& onSettingsChanged,
    bool uiVisible
) {
    if (!uiVisible) {
        return;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
    ImGui::Begin("Wakbg Root", nullptr, flags);

    ImGui::TextUnformatted("Wakbg");
    ImGui::SameLine();
    ImGui::TextDisabled("  Still or animated wallpapers. Close keeps wallpaper running — tray Exit to stop.");
    ImGui::Separator();
    ImGui::Spacing();

    const float sidebarWidth = 168.0f;
    ImGui::BeginChild("Sidebar", ImVec2(sidebarWidth, 0), true);
    const char* items[] = { "Library", "Favorites", "Recent", "Settings", "About" };
    for (int i = 0; i < 5; ++i) {
        if (ImGui::Selectable(items[i], m_libraryState.sidebarIndex == i)) {
            m_libraryState.sidebarIndex = i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("Content", ImVec2(0, 0), false);

    if (m_libraryState.sidebarIndex == 0 || m_libraryState.sidebarIndex == 1 || m_libraryState.sidebarIndex == 2) {
        const float libraryWidth = ImGui::GetContentRegionAvail().x * 0.48f;
        ImGui::BeginChild("LibraryArea", ImVec2(libraryWidth, 0), false);
        if (m_libraryState.sidebarIndex == 0) {
            m_libraryView.Draw(animations, m_libraryState, [&](const AnimationPreset& preset) {
                m_libraryState.selectedPresetId = preset.id;
                onSelectPreset(preset);
            });
        } else if (m_libraryState.sidebarIndex == 1) {
            ImGui::TextUnformatted("Favorites");
            for (const auto& id : animations.GetFavorites()) {
                if (const auto* preset = animations.FindById(id)) {
                    if (ImGui::Selectable(preset->name.c_str(), m_libraryState.selectedPresetId == id)) {
                        m_libraryState.selectedPresetId = id;
                        onSelectPreset(*preset);
                    }
                }
            }
        } else {
            ImGui::TextUnformatted("Recently used");
            for (const auto& id : animations.GetRecent()) {
                if (const auto* preset = animations.FindById(id)) {
                    if (ImGui::Selectable(preset->name.c_str(), m_libraryState.selectedPresetId == id)) {
                        m_libraryState.selectedPresetId = id;
                        onSelectPreset(*preset);
                    }
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();
        const AnimationPreset* selected = animations.FindById(m_libraryState.selectedPresetId);
        m_previewView.Draw(
            selected,
            config.Get().animation,
            selected ? animations.IsFavorite(selected->id) : false,
            previewSrv,
            desktopStatus,
            [&]() {
                if (selected) {
                    onApplyPreset(*selected);
                }
            },
            [&]() {
                if (!selected) {
                    return;
                }
                if (animations.IsFavorite(selected->id)) {
                    animations.RemoveFavorite(selected->id);
                } else {
                    animations.AddFavorite(selected->id);
                }
            },
            onSettingsChanged
        );
    } else if (m_libraryState.sidebarIndex == 3) {
        m_settingsView.Draw(config.Get(), onSettingsChanged);
    } else {
        ImGui::TextUnformatted("Wakbg");
        ImGui::TextWrapped("Close the window to keep the animated wallpaper running in the background.");
        ImGui::TextWrapped("Right-click the tray icon to open the UI again or fully exit.");
    }

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void MainWindow::EndFrame() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool MainWindow::WantsCaptureKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool MainWindow::WantsCaptureMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

LibraryViewState& MainWindow::GetLibraryState() {
    return m_libraryState;
}

}

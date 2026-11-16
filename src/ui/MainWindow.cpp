#include "ui/MainWindow.h"
#include "ui/Theme.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <Windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cmath>

namespace ge {
namespace {

constexpr int kSidebarCount = 5;
const char* kSidebarLabels[kSidebarCount] = { "Library", "Favorites", "Recent", "Settings", "About" };

void OpenExternalUrl(const wchar_t* url) {
    if (url && url[0] != L'\0') {
        ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
    }
}

void DrawAboutPage() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(22.0f, 20.0f));
    ImGui::BeginChild("AboutPanel", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    ImGui::TextUnformatted("Wakbg");
    ImGui::TextDisabled("Live desktop backgrounds for Windows");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Wakbg is a lightweight native app for procedural wallpaper animations. "
        "Gradients and motion run on the GPU — not video loops — so they stay sharp, quiet, and easy on resources."
    );

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextUnformatted("How to use");
    ImGui::Spacing();
    ImGui::BulletText("Browse Library and click a wallpaper to preview it.");
    ImGui::BulletText("Tweak colors, speed, and intensity on the right panel.");
    ImGui::BulletText("Press Apply to Desktop to put it behind your icons.");
    ImGui::BulletText("Close the window anytime — the wallpaper keeps running.");
    ImGui::BulletText("Right-click the tray icon to open the UI again or exit.");
    ImGui::BulletText("After apply, Wakbg can restore your wallpaper after reboot.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextUnformatted("Made by");
    ImGui::Spacing();
    ImGui::TextUnformatted("Hummet Azim");
    ImGui::TextDisabled("Software engineer");
    ImGui::Spacing();
    ImGui::TextWrapped("I built Wakbg for personal use.");

    ImGui::Spacing();
    if (ImGui::Button("Website — hummet.dev", ImVec2(220.0f, 0.0f))) {
        OpenExternalUrl(L"https://www.hummet.dev");
    }
    ImGui::SameLine();
    if (ImGui::Button("LinkedIn", ImVec2(120.0f, 0.0f))) {
        OpenExternalUrl(L"https://www.linkedin.com/in/hummet/");
    }
    ImGui::SameLine();
    if (ImGui::Button("GitHub", ImVec2(120.0f, 0.0f))) {
        OpenExternalUrl(L"https://github.com/hummxt");
    }

    ImGui::Spacing();
    ImGui::TextDisabled("hummet.dev  ·  linkedin.com/in/hummet  ·  github.com/hummxt");

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

}

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

void MainWindow::DrawSidebar(float width) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(IM_COL32(7, 11, 22, 255)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 14.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::BeginChild("Sidebar", ImVec2(width, 0), false, ImGuiWindowFlags_NoScrollbar);

    const float dt = ImGui::GetIO().DeltaTime;
    ImDrawList* draw = ImGui::GetWindowDrawList();

    for (int i = 0; i < kSidebarCount; ++i) {
        ImGui::PushID(i);
        const bool selected = m_libraryState.sidebarIndex == i;
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float itemH = 40.0f;
        const ImVec2 size(ImGui::GetContentRegionAvail().x, itemH);

        const bool pressed = ImGui::InvisibleButton("##nav", size);
        const bool hovered = ImGui::IsItemHovered();
        const bool held = ImGui::IsItemActive();

        float hoverTarget = selected ? 1.0f : (hovered ? 0.72f : 0.0f);
        float pressTarget = held ? 1.0f : 0.0f;
        m_sidebarHover[i] = SmoothApproach(m_sidebarHover[i], hoverTarget, 14.0f, dt);
        m_sidebarPress[i] = SmoothApproach(m_sidebarPress[i], pressTarget, 18.0f, dt);

        const float hover = m_sidebarHover[i];
        const float press = m_sidebarPress[i];
        const float sink = press * 1.0f;

        const ImVec2 p0(pos.x, pos.y + sink);
        const ImVec2 p1(pos.x + size.x, pos.y + itemH + sink);

        if (hover > 0.01f) {
            const int alpha = static_cast<int>(18.0f + hover * 42.0f);
            draw->AddRectFilled(p0, p1, IM_COL32(36, 58, 104, alpha), 10.0f);
        }
        if (selected) {
            draw->AddRectFilled(p0, p1, IM_COL32(42, 74, 132, 70), 10.0f);
            draw->AddRectFilled(
                ImVec2(p0.x, p0.y + 8.0f),
                ImVec2(p0.x + 3.0f, p1.y - 8.0f),
                IM_COL32(110, 168, 255, 255),
                2.0f
            );
        }

        const ImU32 textCol = selected
            ? IM_COL32(245, 248, 255, 255)
            : IM_COL32(170 + static_cast<int>(hover * 50.0f), 180 + static_cast<int>(hover * 40.0f), 200 + static_cast<int>(hover * 40.0f), 255);
        const ImVec2 textSize = ImGui::CalcTextSize(kSidebarLabels[i]);
        draw->AddText(
            ImVec2(p0.x + 16.0f, p0.y + (itemH - textSize.y) * 0.5f),
            textCol,
            kSidebarLabels[i]
        );

        if (pressed) {
            m_libraryState.sidebarIndex = i;
        }
        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void MainWindow::Draw(
    Config& config,
    AnimationManager& animations,
    ID3D11ShaderResourceView* previewSrv,
    const std::function<ID3D11ShaderResourceView*(const std::string&)>& getThumbnail,
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
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(IM_COL32(10, 15, 29, 255)));
    ImGui::Begin("Wakbg Root", nullptr, flags);

    const float sidebarWidth = 176.0f;
    DrawSidebar(sidebarWidth);

    ImGui::SameLine(0.0f, 14.0f);
    ImGui::BeginChild("Content", ImVec2(0, 0), false);

    if (m_libraryState.sidebarIndex == 0) {
        const float libraryWidth = ImGui::GetContentRegionAvail().x * 0.54f;
        ImGui::BeginChild("LibraryArea", ImVec2(libraryWidth, 0), false);
        m_libraryView.Draw(animations, m_libraryState, getThumbnail, [&](const AnimationPreset& preset) {
            m_libraryState.selectedPresetId = preset.id;
            onSelectPreset(preset);
        });
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
    } else if (m_libraryState.sidebarIndex == 1) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));
        ImGui::BeginChild("FavoritesList", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::TextUnformatted("Favorites");
        ImGui::Spacing();
        const auto& favorites = animations.GetFavorites();
        if (favorites.empty()) {
            ImGui::TextDisabled("No favorites yet. Star a wallpaper from the Library.");
        } else {
            for (const auto& id : favorites) {
                if (const auto* preset = animations.FindById(id)) {
                    if (ImGui::Selectable(preset->name.c_str(), m_libraryState.selectedPresetId == id)) {
                        m_libraryState.selectedPresetId = id;
                        m_libraryState.sidebarIndex = 0;
                        onSelectPreset(*preset);
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    } else if (m_libraryState.sidebarIndex == 2) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));
        ImGui::BeginChild("RecentList", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::TextUnformatted("Recently used");
        ImGui::Spacing();
        const auto& recent = animations.GetRecent();
        if (recent.empty()) {
            ImGui::TextDisabled("Nothing recent yet. Apply a wallpaper from the Library.");
        } else {
            for (const auto& id : recent) {
                if (const auto* preset = animations.FindById(id)) {
                    if (ImGui::Selectable(preset->name.c_str(), m_libraryState.selectedPresetId == id)) {
                        m_libraryState.selectedPresetId = id;
                        m_libraryState.sidebarIndex = 0;
                        onSelectPreset(*preset);
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    } else if (m_libraryState.sidebarIndex == 3) {
        m_settingsView.Draw(config.Get(), onSettingsChanged);
    } else {
        DrawAboutPage();
    }

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor();
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

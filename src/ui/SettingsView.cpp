#include "ui/SettingsView.h"
#include "desktop/DesktopHost.h"

#include "imgui.h"

namespace ge {

void SettingsView::Draw(AppConfig& config, const std::function<void()>& onChanged) {
    ImGui::BeginChild("SettingsPanel", ImVec2(0, 0), true);
    ImGui::TextUnformatted("Performance");

    int fps = config.performance.fpsLimit;
    if (ImGui::SliderInt("FPS Limit", &fps, 15, 60)) {
        config.performance.fpsLimit = fps;
        onChanged();
    }

    if (ImGui::SliderFloat("Render Scale", &config.performance.renderScale, 0.75f, 1.0f, "%.2f")) {
        onChanged();
    }
    ImGui::TextDisabled("Desktop wallpaper always uses native screen resolution.");

    if (ImGui::Checkbox("Pause when fullscreen", &config.performance.pauseWhenFullscreen)) {
        onChanged();
    }

    if (ImGui::Checkbox("Power saving", &config.performance.powerSaving)) {
        onChanged();
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Application");

    bool start = config.application.startWithWindows;
    if (ImGui::Checkbox("Start with Windows", &start)) {
        config.application.startWithWindows = start;
        SetStartWithWindows(start);
        onChanged();
    }
    ImGui::TextDisabled("Keeps your wallpaper after reboot.");

    if (ImGui::Checkbox("Wallpaper mode", &config.application.wallpaperMode)) {
        if (config.application.wallpaperMode) {
            config.application.startWithWindows = true;
            SetStartWithWindows(true);
        }
        onChanged();
    }
    ImGui::TextDisabled("Turn off only if you want the default Windows wallpaper back.");

    ImGui::EndChild();
}

}

#include "ui/SettingsView.h"
#include "desktop/DesktopHost.h"

#include "imgui.h"

namespace ge {

void SettingsView::Draw(AppConfig& config, const std::function<void()>& onChanged) {
    ImGui::BeginChild("SettingsPanel", ImVec2(0, 0), true);
    ImGui::TextUnformatted("Performance");

    int fps = config.performance.fpsLimit;
    if (fps < 15) {
        fps = 15;
    }
    if (fps > 240) {
        fps = 240;
    }
    if (ImGui::SliderInt("FPS Limit", &fps, 15, 240)) {
        config.performance.fpsLimit = fps;
        onChanged();
    }
    ImGui::TextDisabled("Default 60. Raise for smoother motion, lower to save power.");

    if (ImGui::SliderFloat("Render Scale", &config.performance.renderScale, 0.5f, 1.0f, "%.2f")) {
        onChanged();
    }
    ImGui::TextDisabled("Lower scale = lighter GPU load. Wallpaper stays sharp enough for desktop use.");

    if (ImGui::Checkbox("Pause when fullscreen", &config.performance.pauseWhenFullscreen)) {
        onChanged();
    }

    if (ImGui::Checkbox("Power saving", &config.performance.powerSaving)) {
        onChanged();
    }
    ImGui::TextDisabled("Caps wallpaper around 30 FPS when enabled.");

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

#include "ui/PreviewView.h"

#include "imgui.h"

#include <algorithm>

namespace ge {

void PreviewView::Draw(
    const AnimationPreset* preset,
    AnimationSettings& animation,
    bool isFavorite,
    ID3D11ShaderResourceView* previewSrv,
    const std::string& desktopStatus,
    const std::function<void()>& onApply,
    const std::function<void()>& onToggleFavorite,
    const std::function<void()>& onSettingsChanged
) {
    ImGui::BeginChild("PreviewPanel", ImVec2(0, 0), true);

    if (!preset) {
        ImGui::TextDisabled("Select an animation to preview");
        ImGui::EndChild();
        return;
    }

    ImGui::TextUnformatted(preset->name.c_str());
    ImGui::TextDisabled("%s", preset->category.c_str());
    ImGui::Spacing();
    ImGui::TextWrapped("%s", preset->description.c_str());
    ImGui::Separator();

    ImGui::TextUnformatted("Motion");
    if (ImGui::SliderFloat("Speed", &animation.speed, 0.05f, 3.0f, "%.2f")) {
        onSettingsChanged();
    }
    if (ImGui::SliderFloat("Intensity", &animation.intensity, 0.05f, 2.0f, "%.2f")) {
        onSettingsChanged();
    }
    if (ImGui::SliderFloat("Brightness", &animation.brightness, 0.2f, 2.0f, "%.2f")) {
        onSettingsChanged();
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Colors");
    ImGui::TextDisabled("Each color point shapes the gradient.");

    bool colorChanged = false;
    colorChanged |= ImGui::ColorEdit4("Color A##ca", animation.colorA.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar);
    colorChanged |= ImGui::ColorEdit4("Color B##cb", animation.colorB.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar);
    colorChanged |= ImGui::ColorEdit4("Color C##cc", animation.colorC.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar);
    colorChanged |= ImGui::ColorEdit4("Color D##cd", animation.colorD.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar);

    if (ImGui::Button("Reset Colors", ImVec2(120, 0))) {
        animation.colorA = preset->colorA;
        animation.colorB = preset->colorB;
        animation.colorC = preset->colorC;
        animation.colorD = preset->colorD;
        colorChanged = true;
    }

    if (colorChanged) {
        onSettingsChanged();
    }

    ImGui::Spacing();
    if (ImGui::Button("Apply to Desktop", ImVec2(160, 0))) {
        onApply();
    }
    ImGui::SameLine();
    if (ImGui::Button(isFavorite ? "Unfavourite" : "Favourite", ImVec2(120, 0))) {
        onToggleFavorite();
    }

    if (!desktopStatus.empty()) {
        ImGui::Spacing();
        ImGui::TextWrapped("%s", desktopStatus.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Live Preview");
    ImGui::Spacing();

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float previewHeight = (std::max)(140.0f, avail.y - 8.0f);
    const ImVec2 previewSize(avail.x, previewHeight);

    if (previewSrv) {
        ImGui::Image(reinterpret_cast<ImTextureID>(previewSrv), previewSize);
    } else {
        ImGui::BeginChild("PreviewFallback", previewSize, true);
        ImGui::TextDisabled("Preview unavailable");
        ImGui::EndChild();
    }

    ImGui::EndChild();
}

}

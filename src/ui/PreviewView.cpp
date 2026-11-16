#include "ui/PreviewView.h"

#include "imgui.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace ge {
namespace {

bool DrawColorSwatch(const char* id, float color[4], float size = 28.0f) {
    bool changed = false;
    ImGui::PushID(id);
    if (ImGui::ColorButton("##swatch", ImVec4(color[0], color[1], color[2], color[3]), ImGuiColorEditFlags_NoTooltip, ImVec2(size, size))) {
        ImGui::OpenPopup("picker");
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(id);
        ImGui::ColorButton("##tip", ImVec4(color[0], color[1], color[2], 1.0f), 0, ImVec2(64, 32));
        ImGui::EndTooltip();
    }
    if (ImGui::BeginPopup("picker")) {
        changed |= ImGui::ColorPicker4("##picker", color, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        ImGui::EndPopup();
    }
    ImGui::PopID();
    return changed;
}

void RandomizeColor(std::array<float, 4>& c) {
    c[0] = static_cast<float>(std::rand() % 1000) / 1000.0f;
    c[1] = static_cast<float>(std::rand() % 1000) / 1000.0f;
    c[2] = static_cast<float>(std::rand() % 1000) / 1000.0f;
    c[3] = 1.0f;
}

}

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
        ImGui::TextDisabled("Select a wallpaper to preview");
        ImGui::EndChild();
        return;
    }

    ImGui::TextUnformatted(preset->name.c_str());
    ImGui::TextDisabled("%s", preset->category.c_str());
    ImGui::Spacing();
    ImGui::TextWrapped("%s", preset->description.c_str());
    ImGui::Separator();

    ImGui::TextUnformatted("Motion");
    bool frozen = animation.speed <= 0.0001f;
    if (ImGui::Checkbox("Freeze as still image", &frozen)) {
        animation.speed = frozen ? 0.0f : (std::max)(preset->defaultSpeed, 0.25f);
        onSettingsChanged();
    }
    if (ImGui::SliderFloat("Speed", &animation.speed, 0.0f, 3.0f, "%.2f")) {
        onSettingsChanged();
    }
    ImGui::TextDisabled(animation.speed <= 0.0001f ? "Frozen still — no animation." : "Animated — set Speed to 0 to freeze.");
    if (ImGui::SliderFloat("Intensity", &animation.intensity, 0.05f, 2.0f, "%.2f")) {
        onSettingsChanged();
    }
    if (ImGui::SliderFloat("Brightness", &animation.brightness, 0.2f, 2.0f, "%.2f")) {
        onSettingsChanged();
    }
    if (ImGui::SliderFloat("Angle", &animation.angle, 0.0f, 360.0f, "%.0f deg")) {
        onSettingsChanged();
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Colors");
    ImGui::TextDisabled("Hover a swatch, click to edit. Swap or randomize freely.");

    bool colorChanged = false;
    colorChanged |= DrawColorSwatch("A", animation.colorA.data());
    ImGui::SameLine();
    colorChanged |= DrawColorSwatch("B", animation.colorB.data());
    ImGui::SameLine();
    colorChanged |= DrawColorSwatch("C", animation.colorC.data());
    ImGui::SameLine();
    colorChanged |= DrawColorSwatch("D", animation.colorD.data());

    colorChanged |= ImGui::ColorEdit4("Color A##ca", animation.colorA.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    colorChanged |= ImGui::ColorEdit4("Color B##cb", animation.colorB.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    colorChanged |= ImGui::ColorEdit4("Color C##cc", animation.colorC.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    colorChanged |= ImGui::ColorEdit4("Color D##cd", animation.colorD.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

    if (ImGui::Button("Swap A/B", ImVec2(88, 0))) {
        std::swap(animation.colorA, animation.colorB);
        colorChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Swap C/D", ImVec2(88, 0))) {
        std::swap(animation.colorC, animation.colorD);
        colorChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Rotate", ImVec2(72, 0))) {
        auto tmp = animation.colorA;
        animation.colorA = animation.colorB;
        animation.colorB = animation.colorC;
        animation.colorC = animation.colorD;
        animation.colorD = tmp;
        colorChanged = true;
    }

    if (ImGui::Button("Randomize", ImVec2(100, 0))) {
        static bool seeded = false;
        if (!seeded) {
            std::srand(static_cast<unsigned>(std::time(nullptr)));
            seeded = true;
        }
        RandomizeColor(animation.colorA);
        RandomizeColor(animation.colorB);
        RandomizeColor(animation.colorC);
        RandomizeColor(animation.colorD);
        colorChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Colors", ImVec2(110, 0))) {
        animation.colorA = preset->colorA;
        animation.colorB = preset->colorB;
        animation.colorC = preset->colorC;
        animation.colorD = preset->colorD;
        animation.angle = preset->defaultAngle;
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
    const float previewHeight = (std::max)(160.0f, avail.y - 8.0f);
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

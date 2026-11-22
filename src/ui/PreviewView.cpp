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
        changed |= ImGui::ColorPicker4(
            "##picker",
            color,
            ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar |
                ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview
        );
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

void ApplyPresetDefaults(const AnimationPreset& preset, AnimationSettings& animation) {
    animation.speed = preset.defaultSpeed;
    animation.intensity = preset.defaultIntensity;
    animation.brightness = preset.defaultBrightness;
    animation.angle = preset.defaultAngle;
    animation.zoom = preset.defaultZoom;
    animation.freq = preset.defaultFreq;
    animation.sharpness = preset.defaultSharpness;
    animation.amplitude = preset.defaultAmplitude;
    animation.waveWidthMod = preset.defaultWaveWidthMod;
    animation.offsetX = preset.defaultOffsetX;
    animation.offsetY = preset.defaultOffsetY;
    animation.localWarpIntensity = preset.defaultLocalWarpIntensity;
    animation.localWarpFreqY = preset.defaultLocalWarpFreqY;
    animation.ditherLevels = preset.defaultDitherLevels;
    animation.ditherScale = preset.defaultDitherScale;
    animation.grainIntensity = preset.defaultGrainIntensity;
    animation.grainSpeed = preset.defaultGrainSpeed;
    animation.contrast = preset.defaultContrast;
    animation.vignetteIntensity = preset.defaultVignetteIntensity;
    animation.vignetteRadius = preset.defaultVignetteRadius;
    animation.colorA = preset.colorA;
    animation.colorB = preset.colorB;
    animation.colorC = preset.colorC;
    animation.colorD = preset.colorD;
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
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));
    ImGui::BeginChild("PreviewPanel", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    if (!preset) {
        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::Indent(4.0f);
        ImGui::TextDisabled("Select a wallpaper to preview");
        ImGui::Unindent(4.0f);
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return;
    }

    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::Indent(4.0f);
    ImGui::TextUnformatted(preset->name.c_str());
    ImGui::Unindent(4.0f);
    ImGui::TextDisabled("%s", preset->category.c_str());
    ImGui::Spacing();
    ImGui::TextWrapped("%s", preset->description.c_str());
    ImGui::Separator();

    bool changed = false;

    if (ImGui::CollapsingHeader("Customizer", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();
        ImGui::TextUnformatted("Motion");
        bool frozen = animation.speed <= 0.0001f;
        if (ImGui::Checkbox("Freeze as still image", &frozen)) {
            animation.speed = frozen ? 0.0f : (std::max)(preset->defaultSpeed, 0.25f);
            changed = true;
        }
        changed |= ImGui::SliderFloat("Speed", &animation.speed, 0.0f, 3.0f, "%.2f");
        ImGui::TextDisabled(animation.speed <= 0.0001f ? "Frozen still — no animation." : "Animated — set Speed to 0 to freeze.");
        changed |= ImGui::SliderFloat("Intensity", &animation.intensity, 0.05f, 2.0f, "%.2f");
        changed |= ImGui::SliderFloat("Brightness", &animation.brightness, 0.2f, 2.0f, "%.2f");
        changed |= ImGui::SliderFloat("Angle", &animation.angle, 0.0f, 360.0f, "%.0f deg");

        ImGui::Spacing();
        if (ImGui::TreeNode("Shape & warp")) {
            changed |= ImGui::SliderFloat("Zoom", &animation.zoom, 0.25f, 3.0f, "%.2f");
            changed |= ImGui::SliderFloat("Frequency", &animation.freq, 0.5f, 12.0f, "%.2f");
            changed |= ImGui::SliderFloat("Sharpness", &animation.sharpness, 0.2f, 6.0f, "%.2f");
            changed |= ImGui::SliderFloat("Amplitude", &animation.amplitude, 0.0f, 2.5f, "%.2f");
            changed |= ImGui::SliderFloat("Wave width", &animation.waveWidthMod, 0.0f, 1.5f, "%.2f");
            changed |= ImGui::SliderFloat("Offset X / time-of-day", &animation.offsetX, -1.0f, 1.0f, "%.2f");
            changed |= ImGui::SliderFloat("Offset Y", &animation.offsetY, -1.0f, 1.0f, "%.2f");
            changed |= ImGui::SliderFloat("Local warp", &animation.localWarpIntensity, 0.0f, 2.0f, "%.2f");
            changed |= ImGui::SliderFloat("Warp frequency Y", &animation.localWarpFreqY, 0.2f, 6.0f, "%.2f");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Atmosphere")) {
            changed |= ImGui::SliderFloat("Contrast", &animation.contrast, 0.4f, 2.0f, "%.2f");
            changed |= ImGui::SliderFloat("Grain", &animation.grainIntensity, 0.0f, 0.25f, "%.3f");
            changed |= ImGui::SliderFloat("Grain speed", &animation.grainSpeed, 0.0f, 120.0f, "%.1f");
            changed |= ImGui::SliderFloat("Vignette", &animation.vignetteIntensity, 0.0f, 1.0f, "%.2f");
            changed |= ImGui::SliderFloat("Vignette radius", &animation.vignetteRadius, 0.1f, 1.0f, "%.2f");
            changed |= ImGui::SliderFloat("Dither levels", &animation.ditherLevels, 0.0f, 32.0f, "%.0f");
            changed |= ImGui::SliderFloat("Dither scale", &animation.ditherScale, 1.0f, 12.0f, "%.1f");
            ImGui::TreePop();
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Colors");
        ImGui::TextDisabled("Hover a swatch, click to edit. Swap or randomize freely.");

        changed |= DrawColorSwatch("A", animation.colorA.data());
        ImGui::SameLine();
        changed |= DrawColorSwatch("B", animation.colorB.data());
        ImGui::SameLine();
        changed |= DrawColorSwatch("C", animation.colorC.data());
        ImGui::SameLine();
        changed |= DrawColorSwatch("D", animation.colorD.data());

        changed |= ImGui::ColorEdit4("Color A##ca", animation.colorA.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        changed |= ImGui::ColorEdit4("Color B##cb", animation.colorB.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        changed |= ImGui::ColorEdit4("Color C##cc", animation.colorC.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        changed |= ImGui::ColorEdit4("Color D##cd", animation.colorD.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

        if (ImGui::Button("Swap A/B", ImVec2(88, 0))) {
            std::swap(animation.colorA, animation.colorB);
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Swap C/D", ImVec2(88, 0))) {
            std::swap(animation.colorC, animation.colorD);
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate", ImVec2(72, 0))) {
            auto tmp = animation.colorA;
            animation.colorA = animation.colorB;
            animation.colorB = animation.colorC;
            animation.colorC = animation.colorD;
            animation.colorD = tmp;
            changed = true;
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
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset all", ImVec2(100, 0))) {
            ApplyPresetDefaults(*preset, animation);
            changed = true;
        }
    }

    if (changed) {
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
    ImGui::PopStyleVar();
}

}

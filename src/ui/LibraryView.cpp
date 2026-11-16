#include "ui/LibraryView.h"

#include "imgui.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>

namespace ge {
namespace {

std::string TitleCase(std::string value) {
    bool cap = true;
    for (char& c : value) {
        if (std::isspace(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
            cap = true;
            continue;
        }
        if (cap) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            cap = false;
        }
    }
    return value;
}

void DrawMiniSwatches(const AnimationPreset& preset, float width) {
    const float size = 10.0f;
    const float gap = 4.0f;
    const ImVec2 start = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const std::array<std::array<float, 4>, 4> colors = { preset.colorA, preset.colorB, preset.colorC, preset.colorD };
    for (int i = 0; i < 4; ++i) {
        const float x = start.x + static_cast<float>(i) * (size + gap);
        const ImVec2 p0(x, start.y);
        const ImVec2 p1(x + size, start.y + size);
        const ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(colors[i][0], colors[i][1], colors[i][2], 1.0f));
        draw->AddRectFilled(p0, p1, col, 2.0f);
    }
    ImGui::Dummy(ImVec2(width, size + 2.0f));
}

}

void LibraryView::Draw(AnimationManager& animations, LibraryViewState& state, const std::function<void(const AnimationPreset&)>& onSelect) {
    ImGui::BeginChild("LibraryList", ImVec2(0, 0), true);

    ImGui::TextUnformatted("Categories");
    const auto categories = animations.GetCategories();
    for (const auto& category : categories) {
        const bool selected = state.selectedCategory == category;
        const std::string label = category == "All" ? category : TitleCase(category);
        if (ImGui::Selectable(label.c_str(), selected)) {
            state.selectedCategory = category;
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Search");
    char buffer[128]{};
    std::snprintf(buffer, sizeof(buffer), "%s", state.searchQuery.c_str());
    if (ImGui::InputText("##search", buffer, sizeof(buffer))) {
        state.searchQuery = buffer;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Wallpapers");

    const auto filtered = animations.Filter(state.selectedCategory, state.searchQuery);
    const float cardWidth = 168.0f;
    const int columns = (std::max)(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (cardWidth + 12.0f)));
    int index = 0;
    for (const auto& preset : filtered) {
        if (index % columns != 0) {
            ImGui::SameLine();
        }

        ImGui::PushID(preset.id.c_str());
        ImGui::BeginGroup();
        const bool selected = state.selectedPresetId == preset.id;
        if (ImGui::Selectable("##card", selected, 0, ImVec2(cardWidth, 96.0f))) {
            state.selectedPresetId = preset.id;
            onSelect(preset);
        }
        const ImVec2 min = ImGui::GetItemRectMin();
        ImGui::SetCursorScreenPos(ImVec2(min.x + 10.0f, min.y + 10.0f));
        ImGui::BeginGroup();
        ImGui::TextUnformatted(preset.name.c_str());
        ImGui::TextDisabled("%s", TitleCase(preset.category).c_str());
        if (preset.defaultSpeed <= 0.0001f) {
            ImGui::TextDisabled("Still");
        } else {
            ImGui::TextDisabled("Animated");
        }
        DrawMiniSwatches(preset, cardWidth - 20.0f);
        ImGui::EndGroup();
        ImGui::EndGroup();
        ImGui::PopID();
        ++index;
    }

    ImGui::EndChild();
}

}

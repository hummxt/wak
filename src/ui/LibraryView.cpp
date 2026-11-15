#include "ui/LibraryView.h"

#include "imgui.h"

#include <algorithm>
#include <cstdio>

namespace ge {

void LibraryView::Draw(AnimationManager& animations, LibraryViewState& state, const std::function<void(const AnimationPreset&)>& onSelect) {
    ImGui::BeginChild("LibraryList", ImVec2(0, 0), true);

    ImGui::TextUnformatted("Categories");
    const auto categories = animations.GetCategories();
    for (const auto& category : categories) {
        const bool selected = state.selectedCategory == category;
        if (ImGui::Selectable(category.c_str(), selected)) {
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
    ImGui::TextUnformatted("Animations");

    const auto filtered = animations.Filter(state.selectedCategory, state.searchQuery);
    const float cardWidth = 180.0f;
    const int columns = (std::max)(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (cardWidth + 12.0f)));
    int index = 0;
    for (const auto& preset : filtered) {
        if (index % columns != 0) {
            ImGui::SameLine();
        }

        ImGui::PushID(preset.id.c_str());
        ImGui::BeginGroup();
        const bool selected = state.selectedPresetId == preset.id;
        if (ImGui::Selectable("##card", selected, 0, ImVec2(cardWidth, 88.0f))) {
            state.selectedPresetId = preset.id;
            onSelect(preset);
        }
        ImGui::SetCursorScreenPos(ImGui::GetItemRectMin());
        ImGui::Dummy(ImVec2(8, 8));
        ImGui::TextUnformatted(preset.name.c_str());
        ImGui::TextDisabled("%s", preset.category.c_str());
        ImGui::EndGroup();
        ImGui::PopID();
        ++index;
    }

    ImGui::EndChild();
}

}
